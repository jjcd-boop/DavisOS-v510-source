# Davis OS v1.56 — GIF/ICO + Dynamic PNG Workspaces

Host milestone. Adds first-frame GIF87a/GIF89a LZW decoding with global/local palettes, transparency and interlacing; ICO decoding for embedded PNG and 32-bit DIB icons; FIFO image-cache eviction instead of hard failure at four cached images; and page-backed PNG inflate/filter workspaces up to the existing 1920x1080 safety ceiling. Animated GIF playback, indexed PNG, Adam7 PNG, progressive JPEG and WebP remain unsupported.
