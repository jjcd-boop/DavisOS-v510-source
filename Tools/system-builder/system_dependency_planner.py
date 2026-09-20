#!/usr/bin/env python3
"""Davis Forge Dependency-Aware System Planner v28.
Plans high-level requests into bounded transformation intents, or orders authenticated
SYSSTAGE transactions into a dependency graph before invoking the atomic v21 builder.
It never fabricates authorization and has no promotion path.
"""
from __future__ import annotations
import argparse, json, pathlib, re, subprocess, sys, tempfile
import system_builder as sb

class Reject(RuntimeError): pass

# Audited transformations exposed by v20. A high-level goal may propose these,
# but every proposed node must later arrive as its own authenticated SYSSTAGE.
RULES = [
 ('desktop.workspace_label',4,re.compile(r'(?:rename|set|change).*workspace(?:\s+label)?.*?(?:to\s+)?["\']?([A-Za-z0-9 _-]{1,24})',re.I)),
 ('desktop.applications_label',4,re.compile(r'(?:rename|set|change).*applications(?:\s+menu)?(?:\s+label)?.*?(?:to\s+)?["\']?([A-Za-z0-9 _-]{1,20})',re.I)),
 ('kernel.scheduler_quantum',1,re.compile(r'(?:set|change).*scheduler\s+quantum(?:\s+ticks)?.*?(\d+)',re.I)),
 ('kernel.audio_max_streams',1,re.compile(r'(?:set|change).*audio\s+max(?:imum)?\s+streams.*?(\d+)',re.I)),
 ('driver.usb_poll_timeout',2,re.compile(r'(?:set|change).*usb\s+(?:poll|event)\s+timeout(?:\s+spins)?.*?(\d+)',re.I)),
 ('boot.gop_status_label',3,re.compile(r'(?:rename|set|change).*gop\s+status(?:\s+label)?.*?(?:to\s+)?["\']?([A-Za-z0-9 _./+-]{1,32})',re.I)),
]
COMP={1:'RING0_KERNEL',2:'RING3_DRIVER',3:'BOOTLOADER',4:'DESKTOP_SYSTEM_SERVICE'}

def split_goal(goal:str):
    # Conservative decomposition: separators only. Ambiguous prose remains one clause
    # and fails closed if it cannot map to exactly one audited transformation.
    return [x.strip(' .') for x in re.split(r'\s*(?:;|\band\b|\bthen\b)\s*',goal,flags=re.I) if x.strip(' .')]

def propose(goal:str):
    nodes=[]
    for clause in split_goal(goal):
        hits=[]
        for name,comp,rx in RULES:
            if rx.search(clause): hits.append((name,comp))
        if len(hits)!=1: raise Reject(f'goal clause must map to exactly one audited transformation: {clause!r}')
        name,comp=hits[0]
        nodes.append({'id':len(nodes)+1,'recipe':name,'component':COMP[comp],'component_id':comp,'requested_change':clause,'authorization':'REQUIRES_AUTHENTICATED_SYSSTAGE'})
    if not 1<=len(nodes)<=8: raise Reject('goal must decompose into 1..8 bounded transformations')
    deps=dependency_edges([n['component_id'] for n in nodes])
    return {'planner':'Davis Forge Dependency-Aware System Planner','version':'28.00','state':'PROPOSAL_ONLY_NOT_AUTHORIZED','goal':goal,'nodes':nodes,'dependencies':[{'before':a+1,'after':b+1} for a,b in deps], 'next_gate':'Each node requires a matching authenticated SYSSTAGE before build.'}

def dependency_edges(comps):
    edges=set()
    # Kernel foundations precede Ring-3 drivers and desktop services.
    for i,a in enumerate(comps):
        for j,b in enumerate(comps):
            if i==j: continue
            if a==1 and b in (2,4): edges.add((i,j))
            # Boot presentation/config is finalized after runtime components.
            if b==3 and a in (1,2,4): edges.add((i,j))
    return edges

def topo(n,edges):
    incoming=[0]*n; out=[[] for _ in range(n)]
    for a,b in edges:
        if b not in out[a]: out[a].append(b); incoming[b]+=1
    q=[i for i in range(n) if incoming[i]==0]; order=[]
    while q:
        i=q.pop(0); order.append(i)
        for j in out[i]:
            incoming[j]-=1
            if incoming[j]==0:q.append(j)
    if len(order)!=n: raise Reject('dependency cycle detected')
    return order

def execute(plan_path:pathlib.Path,source:pathlib.Path,out:pathlib.Path,no_build:bool):
    data=json.loads(plan_path.read_text()); raw=data.get('stages',[])
    if not isinstance(raw,list) or not 2<=len(raw)<=8: raise Reject('execution plan must contain 2..8 authenticated stages')
    paths=[]
    for x in raw:
        p=pathlib.Path(x); p=p if p.is_absolute() else plan_path.parent/p
        paths.append(p.resolve())
    stages=[sb.read_stage(p) for p in paths]
    keys=[(s['digest'],s['request_digest']) for s in stages]
    if len(set(keys))!=len(keys): raise Reject('duplicate SYSSTAGE transaction')
    # Prove each stage maps to a current audited recipe before graph construction.
    targets=[]
    for s in stages:
        patch=sb.synthesize_patch(s,source)
        ts=sb.parse_patch_targets(patch)
        bad=[t for t in ts if not sb.allowed(t,s['component'])]
        if bad: raise Reject('source-boundary violation during planning: '+', '.join(bad))
        targets.append(ts)
    # Same-file transformations are permitted only in original user order; preserve
    # that ordering explicitly to make patch anchoring deterministic.
    edges=set(dependency_edges([s['component'] for s in stages]))
    for i in range(len(stages)):
        for j in range(i+1,len(stages)):
            if set(targets[i]) & set(targets[j]): edges.add((i,j))
    order=topo(len(stages),edges)
    out.mkdir(parents=True,exist_ok=True)
    graph={'planner':'Davis Forge Dependency-Aware System Planner','version':'28.00','state':'AUTHORIZED_GRAPH_READY','nodes':[{'original_index':i+1,'stage':str(paths[i]),'component':stages[i]['component_name'],'request':stages[i]['request'],'targets':targets[i]} for i in range(len(stages))], 'edges':[{'before':a+1,'after':b+1} for a,b in sorted(edges)],'execution_order':[i+1 for i in order]}
    (out/'DEPENDENCY_GRAPH.json').write_text(json.dumps(graph,indent=2)+'\n')
    ordered=out/'ORDERED_PLAN.json'; ordered.write_text(json.dumps({'stages':[str(paths[i]) for i in order]},indent=2)+'\n')
    cmd=[sys.executable,str(pathlib.Path(__file__).with_name('system_plan_builder.py')),'--plan',str(ordered),'--source',str(source),'--out',str(out/'atomic_candidate')]
    if no_build: cmd.append('--no-build')
    p=subprocess.run(cmd,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
    (out/'PLANNER_EXECUTION.log').write_text(p.stdout)
    if p.returncode: raise Reject('atomic builder rejected dependency-ordered plan: '+p.stdout[-1000:])
    print(json.dumps({'ok':True,'order':[i+1 for i in order],'graph':str(out/'DEPENDENCY_GRAPH.json'),'candidate':str(out/'atomic_candidate')},indent=2))

def main():
    ap=argparse.ArgumentParser()
    g=ap.add_mutually_exclusive_group(required=True); g.add_argument('--goal'); g.add_argument('--plan',type=pathlib.Path)
    ap.add_argument('--source',type=pathlib.Path); ap.add_argument('--out',required=True,type=pathlib.Path); ap.add_argument('--no-build',action='store_true')
    a=ap.parse_args()
    try:
        if a.goal:
            a.out.mkdir(parents=True,exist_ok=True); rec=propose(a.goal); p=a.out/'PLAN_PROPOSAL.json'; p.write_text(json.dumps(rec,indent=2)+'\n'); print(json.dumps({'ok':True,'proposal':str(p),'nodes':len(rec['nodes'])},indent=2)); return 0
        if not a.source or not a.source.is_dir(): raise Reject('--source known-good tree is required for execution')
        execute(a.plan,a.source,a.out,a.no_build); return 0
    except (Reject,sb.Reject,ValueError,json.JSONDecodeError) as e:
        a.out.mkdir(parents=True,exist_ok=True); (a.out/'REJECTED.txt').write_text(str(e)+'\n'); print('REJECTED:',e,file=sys.stderr); return 2
if __name__=='__main__': raise SystemExit(main())
