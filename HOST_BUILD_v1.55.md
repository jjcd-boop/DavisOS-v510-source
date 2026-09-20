# Davis OS v1.55 — JPEG + Dynamic Image Surfaces

Host milestone. Adds a bounded baseline JPEG decoder (8-bit SOF0, grayscale and 3-component YCbCr, common 1x1/2x1/1x2/2x2 sampling), JPEG dimension probing, and page-backed browser image pixel surfaces up to 1920x1080. Progressive JPEG remains unsupported. PNG decoding remains limited to 320x240 in this milestone because its inflate/filter workspace is still statically bounded; its final pixel surface is now page-backed.
