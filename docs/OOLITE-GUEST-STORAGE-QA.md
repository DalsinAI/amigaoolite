# AC090 guest qualification reliability — daletop storage pressure, 18 Nov 2026

This records measured **host** conditions while Oolite and GFOS javascript.library qualification shared a dedicated AmigaChrome AC090 68040 guest. The observed guest log stalls are not automatically JavaScript or Foundation defects. These observations are diagnostic, not instructions to restart other teams' labs.

## Measurements (04:44–04:46 UTC, 18 Nov)

- `/proc/pressure/io` reached `some avg10=95.45`, `full avg10=56.06` (a severe fraction of host wall time blocked on I/O); a second sample had some=56.06, full=33.77. PSI varies over time and is not a lifetime benchmark.
- `iostat -dx 1 2` sampled `/dev/sdc` about **90.8% busy** with 245 reads/sec, ~2,404 KiB/sec read bandwidth and 17.43 ms average read wait in its later interval. The device is the 7.3 TB USB rotational bridge-backed disk mounted at `/run/media/da1ek/DevDrive`, containing the isolated AC090 lab.
- Multiple tasks were in Linux uninterruptible `D` waits including `rsync` (`balance_dirty_pages`), `find` (`wait_on_buffer`/`lock_buffer`), `distcc` (`rq_qos_wait`), filesystem journal and writeback workers. These are host-wide symptoms; assigning any one process causal responsibility would require additional telemetry.
- The root partition `/dev/sdg2` is a 465.8 GB SSD at **98% full**, with about 9.7 GB available, not a good staging location for a new guest clone.
- A separate `/mnt/xs1000` 1.8 TB ext4 USB drive has **about 1.3 TB free**, and `/tmp` is 16 GB tmpfs with ~11 GB free; both are candidates for *separately verified* isolated test staging. Do not assume an entire AC090 lab fits into tmpfs or that the USB device offers SSD-like latency without measuring it.

## Consequences for Oolite evidence

- Upstream Oolite `JS_NewRuntime` and arithmetic guest script passed, as did one distinct object/array/string/forced-GC unit. These are independent **actual** exit-code-zero guest tests.
- Heavier native SpiderMonkey stress tests did not return in their allotted observation windows; their stdout prefixes were archived. Under the measured I/O pressure this is **not conclusive** evidence of an interpreter or garbage collector defect. See `docs/OOLITE-MOZJS-GC-STRESS-STATUS.md`.
- Another team is qualifying the canonical `DalsinAI/gfopenbrowser` JavaScriptCore `javascript.library` via the source-of-truth `DalsinAI/gfopensdk` ABI. The library and JavaScriptEngine artifacts were staged but a guest evaluation was not yet confirmed; do not conflate this with the separate pinned SpiderMonkey test runtime. See `docs/OOLITE-JAVASCRIPT-LIBRARY-INTEGRATION.md`.
- Guest tests must run **exclusively**, not overwrite a different team's `ACAgent/run.script`. Retain script, stdout and state before stopping a genuinely hung isolated test. Do not reset another team emulator.

## Next operational gate

Schedule an exclusive low-I/O qualification window, preferably place a freshly copied *isolated* AC090 lab on a verified SSD-backed location if its size and IOPS permit, then rerun original runtime/GC and canonical `javascript.library` tests with a bounded end marker and exit code. Capture before/after I/O PSI and actual guest logs. Avoid copying the whole lab during peak disk saturation or using the nearly-full root SSD. Test a new isolated clone; do not relocate or delete the shared historical lab.
