# Davis OS v26.00 — Forge Recovery Center

v26 connects the protected v25 generation store to the UEFI boot path.

- UEFI displays the last candidate checkpoint and whether it reached HEALTHY.
- K boots the active known-good Davis generation.
- R boots the protected KNOWN_GOOD_v25 snapshot directly, including its Ring-3 application set.
- A short timeout defaults to known-good so unattended boots continue normally.
- If the recovery snapshot is absent, boot fails safe to the active known-good kernel.
- Recovery boot is read-only selection; it does not overwrite or promote anything.
- /Home remains outside system generations.

The protected snapshot is integrity-verifiable by `Tools/system-builder/generation_store.py` on the host. The UEFI boot path itself does not yet implement SHA-256 manifest verification; therefore physical-media tamper resistance is not claimed.

Candidate tests are now operator-selected with C. Merely entering the Recovery Center does not consume/mark a candidate attempt. The ATTEMPTED NVRAM record is written only after C is selected and immediately before the candidate is opened.
