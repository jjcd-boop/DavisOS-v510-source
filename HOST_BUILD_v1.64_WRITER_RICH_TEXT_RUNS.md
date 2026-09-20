# Davis OS v1.64 - Writer Rich Text Runs

Implemented in Davis Writer:
- Per-character rich-text metadata for font family, size, color, style, alignment, and line spacing.
- Mouse text selection with visible selection highlighting.
- Toolbar formatting applies to the selected range; with no selection it controls formatting for newly typed text.
- Writer cursor and insertion point support, including insertion into the middle of the document.
- Backspace deletes the selected range or the character before the insertion point.
- Typing replaces a selected range while preserving independent formatting elsewhere in the document.
- Templates initialize document text using the active Writer format.
- Existing templates, spelling/grammar review, thesaurus, and inserted graphics/media placeholders remain available.

Current limitations:
- Writer still uses a 511-character in-memory document limit.
- Mouse hit-testing/caret placement uses the base raster grid; mixed-size text can make the visual caret approximate.
- Paragraph alignment/spacing are stored per character but rendered using the first character of each line as the paragraph setting.
- Save/open, arbitrary image-byte embedding, external TTF/OTF fonts, clipboard, undo/redo, and Office-compatible import/export remain pending.
- Online video remains a link/embed placeholder rather than a streaming player.

Protected input baseline remains unchanged.
