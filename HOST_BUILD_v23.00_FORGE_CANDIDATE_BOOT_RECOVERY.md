# Davis OS v23.00 — Forge Candidate Boot Validation & Recovery

v23 adds a non-promoting A/B-style validation path. The known-good kernel remains `DAVIS/DAVISKRN.BIN`; a staged candidate lives at `DAVIS/CANDIDATE/DAVISKRN.BIN` with a binary `CANDIDATE.STAGE` identity.

The UEFI loader reads the candidate identity and a Davis-specific UEFI NVRAM record. A never-attempted stage is marked ATTEMPTED before loading the candidate. If the candidate reaches initialized desktop state, the kernel records HEALTHY for the same stage digest. Whether it succeeds or crashes, subsequent boots return to the known-good kernel. There is intentionally no automatic promotion in v23.

This means a broken candidate cannot create a repeated candidate boot loop: the attempt record is written before control leaves firmware. A successful candidate proves only that it reached the desktop acknowledgement point; physical hardware behavior still requires user validation.

`Tools/system-builder/candidate_boot_slot.py` packages a canonically built, digest-matching candidate into the USB candidate slot and refuses validation records that did not pass the canonical build.
