# FridgeChef – C Programming Week 1 · Slide Deck Creation Prompt

> **How to use**: Paste the entire contents of this file as your system prompt (or first user message) when asking Claude to generate the presentation. Claude will follow every specification below without deviation.

---

## 0. Global Constraints

- **Slide format**: 16 : 9 only (1920 × 1080 px logical canvas, or equivalent widescreen proportions in python-pptx / EMU units).
- **Language**: All slide copy is produced in **Korean**. All code blocks remain in their original language (C). Comments in code blocks may be Korean.
- **Font**: **Pretendard only**. No other typeface is permitted on any text element — headings, body, captions, code labels, footnotes, or slide numbers. Available weights: Thin (100), ExtraLight (200), Light (300), Regular (400), Medium (500), SemiBold (600), Bold (700), ExtraBold (800), Black (900). Font files are located in `C:\Users\dbstm\Downloads\Pretendard-1.3.9\public\static\`.
- **Logo**: Place the **Brandlogy logo** (supplied separately via knowledge) in the **bottom-right corner** of every slide at a fixed position: right edge 40px inset, bottom edge 32px inset, height 28px (width proportional). Do not use any other logo.
- **Slide numbers**: Display in the **bottom-left corner**, Pretendard Regular 11pt, color `{colors.steel}`, format `01 / 24` (zero-padded current / total).
- **No empty lower thirds**: Every slide must have substantive content occupying the lower half of the canvas. Use summary callout boxes, syntax highlight tables, key-point bullets, or decorative code fragments rather than leaving white space.

---

## 1. Design System

### 1.1 Color Tokens

| Token | Hex | Role |
|---|---|---|
| `{colors.canvas}` | `#FFFFFF` | Page background, card surface |
| `{colors.surface-soft}` | `#F5F6F8` | Code block background, secondary card |
| `{colors.surface-mid}` | `#EAECF0` | Divider tints, zebra row |
| `{colors.ink-deep}` | `#0A1317` | Primary headline text |
| `{colors.ink}` | `#1C2B33` | Body and secondary text |
| `{colors.charcoal}` | `#3A4A54` | Tertiary body, code-label text |
| `{colors.slate}` | `#5A6E7A` | Section header, supporting copy |
| `{colors.steel}` | `#8A9BA6` | Captions, footer, slide numbers |
| `{colors.stone}` | `#B4C1C9` | Disabled, de-emphasized |
| `{colors.hairline}` | `#DDE3E8` | Card borders, table dividers |
| `{colors.accent}` | `#1A6FE8` | Chapter accent bar, active keyword highlight |
| `{colors.accent-soft}` | `#E8F0FE` | Keyword chip background |
| `{colors.code-bg}` | `#1E2A32` | Dark code-block panel background |
| `{colors.code-text}` | `#D4E0EA` | Default monospace text inside dark panels |
| `{colors.code-keyword}` | `#5BA4FC` | C keywords (`int`, `char`, `struct`, `typedef`, etc.) |
| `{colors.code-type}` | `#79D7A8` | Type names and typedefs |
| `{colors.code-comment}` | `#7A8FA0` | Inline comments |
| `{colors.code-string}` | `#F4B860` | String literals |
| `{colors.code-number}` | `#F08080` | Numeric literals |
| `{colors.warning}` | `#F4C842` | Highlight badge, "주의" callout |
| `{colors.success}` | `#3DBE7A` | "정상 동작" badge |
| `{colors.critical}` | `#E84040` | Error badge, destructive label |

### 1.2 Typography

All roles use **Pretendard**. OpenType feature `{features.ligatures}` off; `{features.kern}` on.

| Token | Size | Weight | Line-height | Letter-spacing | Use |
|---|---|---|---|---|---|
| `{type.chapter-label}` | 11pt | 600 | 1.20 | +0.12em | Chapter eyebrow label (e.g., "CHAPTER 02") |
| `{type.title}` | 32pt | 700 | 1.15 | -0.02em | Slide main title |
| `{type.subtitle}` | 18pt | 400 | 1.40 | 0 | Slide subtitle / section lead |
| `{type.body-lg}` | 14pt | 400 | 1.60 | -0.01em | Primary body text in explanation panels |
| `{type.body-sm}` | 11pt | 400 | 1.55 | -0.01em | Secondary body, bullet lists |
| `{type.body-bold}` | 14pt | 700 | 1.60 | -0.01em | Inline emphasis, key terms |
| `{type.caption}` | 9pt | 400 | 1.40 | 0 | Footnotes, source labels |
| `{type.code}` | 12pt | 400 | 1.65 | 0 | Monospace code inside dark panels (Pretendard Regular acts as fallback mono) |
| `{type.code-label}` | 9pt | 700 | 1.20 | +0.06em | Line-number prefix, function name chips above code blocks |
| `{type.badge}` | 9pt | 700 | 1.20 | +0.04em | Status badges and callout pills |
| `{type.slide-number}` | 11pt | 400 | 1.20 | 0 | Bottom-left slide counter |

### 1.3 Spacing

Base unit 8px. All padding and gap values must be multiples of 8.

| Token | Value | Typical use |
|---|---|---|
| `{sp.xs}` | 8px | Icon-label gap, inline chip padding |
| `{sp.sm}` | 16px | Card inner padding (tight) |
| `{sp.md}` | 24px | Standard card inner padding |
| `{sp.lg}` | 32px | Section vertical gap |
| `{sp.xl}` | 48px | Between major layout zones |
| `{sp.xxl}` | 64px | Top margin on title slides |

### 1.4 Shapes & Elevation

| Token | Value | Use |
|---|---|---|
| `{rounded.sm}` | 4px | Badge / chip |
| `{rounded.md}` | 8px | Card, table cell, callout box |
| `{rounded.lg}` | 12px | Code panel |
| `{rounded.xl}` | 16px | Section divider card |
| `{rounded.full}` | 9999px | Pill label |
| `{shadow.card}` | `0 2px 8px rgba(10,19,23,0.10)` | Raised card |
| `{shadow.panel}` | `0 4px 20px rgba(10,19,23,0.14)` | Floating panel |

---

## 2. Fixed Layout Grid

Every slide shares an identical anchor grid. **Chapter label, title, and subtitle must land on the same coordinates on every slide.** Deviations break the rhythmic continuity of the deck.

```
┌─────────────────────────────────────────────────────────┐  ← y = 0
│  SAFE MARGIN  (left 64px, right 64px, top 48px)         │
│                                                          │
│  ┌── CHAPTER LABEL ──────────────────────────────────┐  │  y = 48px
│  │  {type.chapter-label}  color={colors.accent}      │  │  h = 20px
│  └────────────────────────────────────────────────────┘  │
│                                                          │
│  ┌── SLIDE TITLE ─────────────────────────────────────┐  │  y = 76px
│  │  {type.title}  color={colors.ink-deep}             │  │  h = 44px
│  └────────────────────────────────────────────────────┘  │
│                                                          │
│  ┌── SUBTITLE ────────────────────────────────────────┐  │  y = 128px
│  │  {type.subtitle}  color={colors.slate}             │  │  h = 30px
│  └────────────────────────────────────────────────────┘  │
│                                                          │
│  ──────────────── CONTENT ZONE ─────────────────────── │  y = 172px
│  (all variable content: code panels, bullets, tables)    │  h = 836px
│                                                          │
│  ── FOOTER BAR ─────────────────────────────────────── │  y = 1020px
│  Slide number (left)          Brandlogy logo (right)     │  h = 28px
└─────────────────────────────────────────────────────────┘  ← y = 1080
```

- The **accent bar**: a 3px vertical bar in `{colors.accent}` sits flush-left, spanning from the chapter label top to the subtitle bottom (y=48 → y=158), x=64px.
- The chapter label, title, and subtitle are offset **72px from the left edge** (8px right of the accent bar).

---

## 3. Slide Type Library

### TYPE A — Cover Slide
One per deck. Full-bleed `{colors.ink-deep}` background. Centered layout.

- Project name in `{type.title}` × 2.5 scale (80pt), white, centered.
- Subtitle line (team / week / date) in `{type.subtitle}`, `{colors.stone}`, centered below.
- Brandlogy logo centered, 40px from bottom.
- Horizontal rule in `{colors.accent}` 2px, 480px wide, centered between title and subtitle.
- No slide number on cover.

### TYPE B — Chapter Title Slide
One per chapter. Split-background: left 40% `{colors.accent}` fill, right 60% `{colors.canvas}`.

- Left panel: chapter number large (96pt, Pretendard ExtraBold, white) + chapter name below (24pt, white).
- Right panel: chapter overview in 3–4 bullet points (`{type.body-lg}`), preceded by section label "이 챕터에서 다룰 내용" in `{type.chapter-label}` `{colors.accent}`.
- Brandlogy logo bottom-right on white panel.

### TYPE C — Code + Explanation Split Slide *(primary content type)*
Used for every code segment. The content zone (below subtitle) is divided into two equal vertical columns separated by a 1px `{colors.hairline}` divider.

```
┌─────────────────────────────────────────────────────────┐
│  Chapter label  /  Title  /  Subtitle  (fixed header)   │
├──────────────────────────┬──────────────────────────────┤
│  LEFT PANEL  (50%)       │  RIGHT PANEL  (50%)          │
│                          │                              │
│  Dark code block         │  Function/segment name       │
│  `{colors.code-bg}`      │  ─────────────────────────  │
│  Pretendard Regular 12pt │  • 역할 (Purpose)            │
│  Syntax-highlighted      │  • 입력값 / 반환값           │
│  Line numbers in         │  • 핵심 동작 설명            │
│  `{colors.code-comment}` │  • 주의사항 또는 TODO        │
│                          │                              │
│  ┌─ function chip ─────┐ │  Callout box if applicable   │
│  │ function name label │ │  (`{colors.accent-soft}`)    │
│  └─────────────────────┘ │                              │
└──────────────────────────┴──────────────────────────────┘
│  BOTTOM FILL ZONE — keyword table OR flow summary        │
└─────────────────────────────────────────────────────────┘
```

**Bottom fill zone rules** (must not be empty):
- If the code uses ≥ 3 distinct C keywords or standard-library calls → render a **Keyword Reference Table**: two columns `키워드 / 함수명` | `설명`, max 5 rows, `{colors.surface-soft}` background, `{rounded.md}`.
- If the code is primarily a data structure definition → render a **Memory Layout Diagram**: labeled struct fields as stacked rectangles with byte sizes.
- If the code is a menu / output function → render a **실행 화면 미리보기** box: monospace text in `{colors.code-bg}` showing example terminal output.
- If none of the above apply → render a **핵심 포인트 카드**: 2–3 bullet takeaways in `{type.body-bold}` inside a `{colors.accent-soft}` rounded card.

### TYPE D — Concept / Overview Slide
For slides without code (table of contents, architecture diagram, project summary).

- Full-width content zone; use 2- or 3-column card grids with `{shadow.card}`.
- Each card: `{rounded.md}`, `{colors.canvas}`, 1px `{colors.hairline}` border.
- Bottom zone: summary sentence in `{type.subtitle}` `{colors.slate}` centered.

### TYPE E — Summary / Wrap-Up Slide
One per chapter. Lists all functions/segments introduced in the chapter.

- Left: numbered list of covered items, `{type.body-lg}`.
- Right: 2×2 status badge grid (`{colors.success}` = 구현 완료, `{colors.warning}` = 예정/TODO).
- Bottom: "다음 챕터 예고" teaser in `{colors.accent-soft}` pill card.

---

## 4. Chapter Structure

Generate the following chapters and slides **in order**. Use TYPE B for chapter openers, TYPE C for every code segment, TYPE E for chapter closers, and TYPE D where noted.

> **Code coverage rule**: Every named function and every `#define` / `typedef` block in the source must appear on at least one TYPE C slide. If a chapter has more code than fits on two TYPE C slides, add additional TYPE C slides until all code is covered.

---

### CHAPTER 01 · 프로젝트 개요
**Slides**: Cover (TYPE A) → Chapter Title (TYPE B) → 2× TYPE D → Chapter Close (TYPE E)

TYPE D slides cover:
1. **프로젝트 소개**: FridgeChef 개요, 목표, 팀 정보, 개발 환경 (Visual Studio, Windows, C)
2. **프로그램 흐름도**: 메인 루프 플로우차트 (메뉴 → switch → 각 기능 → 루프 복귀)

---

### CHAPTER 02 · 전처리 & 헤더 설정
**Slides**: Chapter Title (TYPE B) → TYPE C slides → Chapter Close (TYPE E)

TYPE C slides (one per block, add more if needed):

| Slide | Code Segment | Left Panel Focus |
|---|---|---|
| 02-01 | `#define _CRT_SECURE_NO_WARNINGS` + all `#include` directives | Preprocessor macros |
| 02-02 | All `#define` constants (`MAX_ITEMS`, `MAX_RECIPES`, `MAX_ING`, `IMMINENT`, `FILENAME`) | Manifest constants |

---

### CHAPTER 03 · 자료구조 정의
**Slides**: Chapter Title (TYPE B) → TYPE C slides → Chapter Close (TYPE E)

| Slide | Code Segment |
|---|---|
| 03-01 | `typedef struct { … } Item;` — full struct definition with field annotations |
| 03-02 | `typedef struct { … } Recipe;` — full struct definition |
| 03-03 | Global variable declarations (`inventory[]`, `itemCount`, `recipes[]`, `recipeCount`) |

---

### CHAPTER 04 · 유틸리티 함수
**Slides**: Chapter Title (TYPE B) → TYPE C slides → Chapter Close (TYPE E)

| Slide | Code Segment |
|---|---|
| 04-01 | `printLine()` function |
| 04-02 | `flushInput()` function (static helper) |
| 04-03 | `countImminent()` — current stub + intended future logic description |
| 04-04 | `showMenu()` — full function including conditional `imm` branch |

---

### CHAPTER 05 · 파일 입출력
**Slides**: Chapter Title (TYPE B) → TYPE C slides → Chapter Close (TYPE E)

| Slide | Code Segment |
|---|---|
| 05-01 | `saveItems()` — `fopen("w")`, `fprintf` loop, `fclose` |
| 05-02 | `loadItems()` — `fopen("r")`, `fscanf` loop with pipe-delimited format string, `fclose` |

---

### CHAPTER 06 · 재료 목록 출력
**Slides**: Chapter Title (TYPE B) → TYPE C slides → Chapter Close (TYPE E)

| Slide | Code Segment |
|---|---|
| 06-01 | `listItems()` — empty-state branch |
| 06-02 | `listItems()` — table header printf block |
| 06-03 | `listItems()` — for-loop body (row printf) + footer + `flushInput` / `getchar` |

---

### CHAPTER 07 · 재료 추가 기능 *(핵심 챕터)*
**Slides**: Chapter Title (TYPE B) → TYPE C slides → Chapter Close (TYPE E)

| Slide | Code Segment |
|---|---|
| 07-01 | `addItem()` — function signature + full-inventory guard + `flushInput()` call |
| 07-02 | `addItem()` — name input block (`fgets` + `strcspn` + empty-string guard) |
| 07-03 | `addItem()` — category menu print + `scanf` + `switch-case` block |
| 07-04 | `addItem()` — location menu print + `scanf` + `switch-case` block |
| 07-05 | `addItem()` — quantity input + `flushInput()` before expiry |
| 07-06 | `addItem()` — expiry `fgets` + `strcspn` + default "미정" fallback |
| 07-07 | `addItem()` — `inventory[itemCount++] = newItem;` + `saveItems()` + success print + `getchar()` |

---

### CHAPTER 08 · 미구현 기능 (스텁)
**Slides**: Chapter Title (TYPE B) → TYPE C slides → Chapter Close (TYPE E)

| Slide | Code Segment |
|---|---|
| 08-01 | `deleteItem()` stub — explain planned logic (name/index search + array shift) |
| 08-02 | `checkAlert()` stub — explain planned logic (`time.h` date diff vs `IMMINENT`) |
| 08-03 | `showRecipes()` stub — explain planned logic (ingredient intersection matching) |

---

### CHAPTER 09 · main() 함수
**Slides**: Chapter Title (TYPE B) → TYPE C slides → Chapter Close (TYPE E)

| Slide | Code Segment |
|---|---|
| 09-01 | `main()` — variable declarations + `SetConsoleOutputCP` / `SetConsoleCP` block |
| 09-02 | `main()` — `loadItems()` call + `while(1)` loop + `system("cls")` + `showMenu()` + `scanf` |
| 09-03 | `main()` — full `switch` block (cases 1–5) |
| 09-04 | `main()` — `case 0` exit confirmation + `default` error handling |

---

### CHAPTER 10 · 실행 시연 & 회고
**Slides**: Chapter Title (TYPE B) → 2× TYPE D → Chapter Close (TYPE E)

TYPE D slides:
1. **실행 화면 시연**: Terminal screenshots / ASCII mockups of the menu, addItem flow, listItems table
2. **개발 회고 & 향후 계획**: What was implemented (Week 1), what remains (deleteItem, checkAlert, showRecipes, file load on startup confirmation, input validation)

---

## 5. Code Block Formatting Rules

1. **Dark panel**: background `{colors.code-bg}` (#1E2A32), border-radius `{rounded.lg}` (12px), padding `{sp.sm}` (16px) all sides.
2. **Line numbers**: right-aligned in a 28px-wide gutter, color `{colors.code-comment}`, separated from code by a 1px `{colors.charcoal}` vertical rule.
3. **Syntax coloring** (apply consistently):
   - Keywords (`int`, `char`, `void`, `return`, `if`, `else`, `while`, `for`, `switch`, `case`, `break`, `default`, `typedef`, `struct`, `static`, `ifdef`, `define`, `include`) → `{colors.code-keyword}`
   - Type names and typedefs (`Item`, `Recipe`, `FILE`) → `{colors.code-type}`
   - String literals (anything between `"…"`) → `{colors.code-string}`
   - Numeric literals → `{colors.code-number}`
   - Comments (single-line `/* … */`) → `{colors.code-comment}`
   - All other code → `{colors.code-text}`
4. **Function name chip**: above each code panel, render a pill label (`{rounded.full}`, background `{colors.accent}`, text white, `{type.code-label}`) showing the function name (e.g., `addItem()`).
5. **If code is too long for one slide**: split naturally at a logical boundary (end of a conditional block, end of a loop), add a continuation chip "… 이어서" at the bottom of the left panel, and open the next TYPE C slide with the continuation.

---

## 6. Explanation Panel Rules (Right Column)

Every explanation panel must contain **all four** of the following sections (use short horizontal rules to separate them):

```
┌────────────────────────────────┐
│  🔷 역할                       │  1–2 sentences: what this code does
│  ─────────────────────────     │
│  🔷 입력 / 출력 / 반환값       │  parameters → return value or side effects
│  ─────────────────────────     │
│  🔷 핵심 동작                  │  2–4 bullets: step-by-step logic
│  ─────────────────────────     │
│  🔷 주의사항 / 개선 예정       │  1–2 bullets: edge cases, TODO items
└────────────────────────────────┘
```

- Section headers in `{type.body-bold}` `{colors.accent}`.
- Body text in `{type.body-lg}` `{colors.ink}`.
- Bullets use `▸` as the bullet character, color `{colors.accent}`.

---

## 7. Quality Checklist (apply before finalizing each slide)

- [ ] Chapter label, title, subtitle are at their fixed y-coordinates.
- [ ] Brandlogy logo is in the bottom-right corner on every slide (except cover uses centered logo).
- [ ] Slide number is in the bottom-left corner (except cover).
- [ ] No slide has an empty lower third — bottom fill zone is populated.
- [ ] Only Pretendard is used; no fallback system font appears.
- [ ] Every `#define`, `typedef`, and named function in the source appears on at least one TYPE C slide.
- [ ] Code panels use dark background; explanation panels use white/soft background.
- [ ] The 3px accent bar is present and aligns with the chapter label top on every non-cover slide.
- [ ] All four explanation sections (역할 / 입력·출력 / 핵심 동작 / 주의사항) are present on every TYPE C right panel.
