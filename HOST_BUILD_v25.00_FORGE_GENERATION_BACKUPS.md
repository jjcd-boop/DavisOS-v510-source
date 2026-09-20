# Davis OS v25.00 — Forge Protected Generation Backups

v25 adds a host-side Generation Store. A generation is a cryptographically manifested copy of the Davis system surface: UEFI bootloader, both known-good kernel locations, and the complete Ring-3 application tree. `/Home`, `DAVIS/CANDIDATE`, and `DAVIS/RECOVERY` are deliberately excluded.

Commands `snapshot`, `verify`, and `restore` are fail-closed. Restore writes only to a fresh staging directory and records `STAGED_NOT_PROMOTED`; it never overwrites the live system. This preserves operator authority.

The USB-ready v25 image carries a protected KNOWN_GOOD generation under `DAVIS/RECOVERY/KNOWN_GOOD_v25`. Its manifest contains per-file SHA-256 hashes plus a manifest digest. This is a recovery copy, not a claim of immutable media: FAT32 itself has no cryptographic write protection. A truly immutable factory copy requires separate read-only media/partition or firmware-enforced policy.

Physical recovery boot from the generation store is not yet automatic. Candidate one-shot recovery remains as in v24. The generation store is the durable backup substrate for a later Recovery Center and promotion/rollback transaction.
