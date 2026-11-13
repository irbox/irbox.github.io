---
title: Theming
description: eNature colorimetry, positive contrast polarity, APCA contrast validation, 34.4 arcmin angular typography, and clinical ergonomic protocol.
category: Customizing
---

**eNature** is the native, default theme system for this site. Grounded in retinal neurobiology, perifoveal receptive field electrophysiology, and optical coherence tomography (OCT) findings (*Aleman, Wang, & Schaeffel 2018, Scientific Reports 8:10840*; *Swiatczak & Schaeffel 2022, TVST*; *Ingrassia et al. 2024, Vision Res*; *Fan et al. 2024, Sensors*), eNature is already applied out-of-the-box across every page. **There is no extra button to apply the theme**; the only user option is switching between the **dark** and **light** version of the theme.

- **eNature Dark (`data-theme='dark'` / `data-enature='dark'`) [Default]**: The primary biological baseline. Maximum contrast (16.48:1 base / 15.33:1 raised), deep ocular substrate (`#0A0F14`) activating the retinal ON-pathway through positive contrast polarity, promoting choroidal thickening (+10 µm) and inhibiting scleral axial elongation.
- **eNature Light (`data-theme='light'` / `data-enature='light'`)**: Daylight and high-ambient fallback (16.34:1 contrast, `#F5F7F9` base, `#121A22` text). Engineered for proofreading (*Piepenbrock et al. 2013*), high-glare window workstations, and older readers. **Explicitly documented as NOT myopia-informed** (positive contrast polarity is not active).

Use the topbar theme toggle button in the header (or press **`T`** anywhere on your keyboard) to switch between **eNature Dark** and **eNature Light**.

---

## 1. Research Synthesis & Physiological Verification

Cross-referencing peer-reviewed vision science clarifies the neurobiological constraints governing eNature:

### Perifoveal Receptive Fields Drive the 34.4 Arcmin Threshold
Building on the original 1-hour TV reading experiment by *Aleman, Wang & Schaeffel (2018)* on *Nature Scientific Reports*, *Swiatczak & Schaeffel (2022, TVST)* demonstrated that inverted text shortened myopic eyes by **$-10.2 \pm 9.5\ \mu\text{m}$ after 30 minutes**, whereas small $13.75\text{ arcmin}$ text produced **no significant effect** ($p = 0.09$).

As reviewed in Swiatczak's *2025 ZEISS Young Investigator Award Lecture* (*Ophthalmic & Physiological Optics*) and pattern electroretinography (PERG) studies by *Wagner & Strasser (2023, IOVS)*:
- Retinal sensitivity to contrast polarity in myopes shifts strongly toward the **$6^\circ$ to $12^\circ$ perifoveal zone**.
- Larger perifoveal receptive fields require **low spatial frequencies (~5 cycles/deg)**—directly matching **$\sim 34.4\text{ arcmin}$ letter height**—to reduce continuous OFF-channel drive.

### Blue-Channel Preservation Rules Out Warm/Amber Tints
*Ingrassia, Swiatczak & Schaeffel (2024, Vision Research)* discovered that adding a digital "red-in-focus" filter to inverted text **reversed the protective benefit and caused emmetropic eyes to lengthen**:
- Removing short-wavelength blue spatial detail deprives the retina of longitudinal chromatic aberration (LCA) signals needed to thicken the choroid.
- Independently, *Fan et al. (2024, MDPI Sensors)* proved that **red text under dark mode causes the highest visual fatigue and worst cognitive performance**, whereas **yellow and white text cause the least visual fatigue**.

### Biomarker Caveats & Positive-Polarity Trade-Offs
- Both the **IMI Dynamic Choroid Taskforce** and the **IMI 2025 Clinical Digest** caution that short-term human choroidal shifts ($5\text{–}30\ \mu\text{m}$, or $2\text{–}3\%$ of total thickness) are **not validated predictors of long-term refractive control**.
- *Wagner, Settele & Strasser (2025, IOVS)* noted that inverted text mainly dampens OFF-pathway overstimulation rather than selectively boosting ON-responses.
- Because *Piepenbrock et al. (2013)* established better proofreading accuracy with dark-on-light polarity in clear eyes, **eNature Light remains essential as a non-myopia-targeted fallback**.

---

## 2. Colorimetric & Typographic Audit

Auditing colorimetry across WCAG 2.1 relative luminance and polarity-sensitive APCA (Accessible Perceptual Contrast Algorithm, $L_c$) addresses four critical engineering gaps:

1. **Fixing Dark-Mode Syntax & Error Starvation on Raised Surfaces:**
   WCAG 2.x inflates dark-mode contrast. On raised `<pre>`/`<code>` surfaces (`#101820`), conventional tokens produced APCA scores of just $L_c\ -47.2$ (red), $-51.3$ (purple), and $-36.4$ (muted), failing the $|L_c| \ge 60$ fluent syntax and $|L_c| \ge 45$ comment thresholds. eNature elevates accent lightness while retaining cool-neutral blue subpixels so **every syntax accent achieves $|L_c| \ge 60.6$ on raised code blocks** without triggering halation.
2. **Yellow Accent Priority & Red Quarantine:**
   Following *Fan et al. (2024)*, eNature promotes `--enature-accent-yellow` (`#F2CC6B`, $L_c\ -77.2$) to active cursors, search matches, and highlights, while quarantining red (`#F69B96`) strictly to error states.
3. **Subpixel Stem-Thinning Protection:**
   Light glyphs on dark backgrounds appear thinner under grayscale antialiasing. eNature sets `-webkit-font-smoothing: auto`, adds `font-size-adjust: 0.52`, and nudges dark-mode body weight to `450` on variable fonts to preserve stroke width.
4. **Lowercase x-Height vs. Cap-Height Calibration:**
   At 96 DPI, a $34.4\text{ arcmin}$ target with a $0.70\text{em}$ cap height yields exact CSS sizes of `16.2px` (30 cm), `21.6px` (40 cm), `27.0px` (50 cm), `32.4px` (60 cm), and `37.8px` (70 cm). Because body text is mostly lowercase ($\sim 0.52\text{em}$ x-height), eNature cap-height presets ensure comfortable legibility across viewing ranges.

### APCA Contrast Verification (Measured on Raised `pre`/`code` Surfaces)

| Token Role | Variant | Hex | WCAG Ratio (Base / Raised) | APCA ($L_c$ Raised) | Blue Subpixel (0–255) |
| :--- | :--- | :---: | :---: | :---: | :---: |
| **Body Text (`primary`)** | eNature Dark | `#E9EEF3` | 16.48:1 / 15.33:1 | $-95.3$ (Body Pass) | 243 (Cool-Neutral D65) |
| **Body Text (`primary`)** | eNature Light | `#121A22` | 16.34:1 / 15.22:1 | $+94.7$ (Proofreading) | 34 (Light Fallback) |
| **Muted / Comments** | eNature Dark | `#8C9DAB` | 6.90:1 / 6.41:1 | $-47.0$ (Pass $\ge 45$) | 171 |
| **Error Red (`accent-red`)** | eNature Dark | `#F69B96` | 9.21:1 / 8.56:1 | $-60.6$ (Pass $\ge 60$) | 150 |
| **Link Blue (`accent-blue`)** | eNature Dark | `#79BDFF` | 9.65:1 / 8.98:1 | $-62.9$ (Pass $\ge 60$) | 255 |
| **Syntax Purple (`purple`)** | eNature Dark | `#C2A8F3` | 9.34:1 / 8.68:1 | $-61.1$ (Pass $\ge 60$) | 243 |
| **Fatigue-Safe Yellow** | eNature Dark | `#F2CC6B` | 12.48:1 / 11.61:1 | $-77.2$ (Optimal) | 107 |

---

## 3. Reading-Size Presets (~34.4 Arcmin Rule)

Typography subtends approximately $34.4\text{ arcmin}$ of visual angle to activate perifoveal ON-ganglion cells across common physical distances:

| Viewing Distance | Cap Height (0.70em, Default) | x-Height (0.52em, Strict) | Device / Context |
| :---: | :---: | :---: | :--- |
| **30 cm** | `16.2px` | `21.8px` | Handheld smartphone (minimum safe distance) |
| **40 cm** | `21.6px` | `29.1px` | **Preferred reading distance** (tablets, close laptops) |
| **50 cm** | `27.0px` | `36.4px` | Standard laptop distance on desk |
| **60 cm** | `32.4px` | `43.6px` | Standard external desktop monitor (24″–27″) |
| **70 cm** | `37.8px` | `50.9px` | Large or deep workstation monitor |

---

## 4. Age-Stratified Ergonomic & Behavioural Protocol

Aligned with the International Myopia Institute (IMI 2025 Clinical Digest):

| Age Group | Suggested Variant | Distance & CSS Size Preset | Primary Evidence & Clinical Priority |
| :--- | :--- | :--- | :--- |
| **Under 3 Years** | None (No Screens) | N/A | Follow WHO, AAP, and Nov 2025 public health guidance avoiding screen exposure. |
| **Children (3–14)** | eNature Dark | $\ge 40\text{ cm}$ (`21.6px` cap / `29.1px` x-h) | $\sim 2\text{ hrs/day}$ outdoor daylight is the strongest protective factor; theme is an untested extra. |
| **Teens & Young Adults (15–25)** | eNature Dark | $40\text{–}60\text{ cm}$ (`21.6px`–`32.4px`) | Closest match to lab cohorts; stop continuous near work at 30 min for a 5-min distance break. |
| **Adults (25–40)** | eNature Dark or Light | $50\text{–}60\text{ cm}$ (`27.0px`–`32.4px`) | Progression usually slows below $0.10\text{ D/yr}$; maintain $500\text{–}1000\text{ lux}$ room lighting. |
| **Adults (40–60)** | eNature Dark or Light | $50\text{–}70\text{ cm}$ (`27.0px`–`37.8px`) | Presbyopia and uncorrected astigmatism dominate comfort; prioritize accurate near-work lenses. |
| **Older Adults (60+)** | eNature Light (clear) / Dark | $40\text{–}60\text{ cm}$ (`21.6px`–`32.4px+`) | Clear lenses read better on eNature Light; lens scatter or cataract often favours eNature Dark. |

- **Room Lighting & Screen Brightness:** Maintain indoor lighting around $500\text{–}1000\text{ lux}$ and moderate screen brightness ($35\text{–}86\text{ cd/m}^2$), avoiding dark displays in pitch-black rooms.
- **Keep Color Temperature Neutral:** Disable warm/amber night filters during eNature Dark reading sessions so blue-channel LCA cues stay intact (*Ingrassia et al. 2024*).
- **Pseudomyopia Warning Signs:** Temporary distance blur or headaches after sustained smartphone or monitor reading signal functional ciliary focusing spasm rather than immediate axial growth. Schedule a cycloplegic refraction with an eye care professional rather than self-diagnosing.

---

## 5. Clinical Caveats & Disclaimers

> [!IMPORTANT]
> **Precautionary Defaults, Not Medical Treatment:**
> The eNature color palette and angular typography presets are ergonomic design defaults based on short-term laboratory optical coherence tomography (OCT) findings. They are not medical treatments, do not cure refractive errors, and have not been tested in randomized long-term human myopia prevention trials.

> [!WARNING]
> **Accommodative Spasm ("Pseudomyopia"):**
> If your distance vision becomes blurry following prolonged near work and recovers over minutes to hours, or if computer work is accompanied by frontal headaches and eyestrain, you may be experiencing **ciliary accommodative spasm (pseudomyopia)**.
>
> A color palette cannot treat or release accommodative spasm. Pseudomyopia requires a comprehensive eye examination by an optometrist or ophthalmologist, including **cycloplegic refraction** (eye drops that temporarily relax the ciliary muscle).

---

## 6. Design Tokens & Implementation

```css
/* eNature Dark (Default Baseline) */
:root, [data-theme='dark'], [data-enature='dark'] {
  --enature-background-base: #0A0F14;
  --enature-background-raised: #101820;
  --enature-background-overlay: #17222C;
  --enature-background-highlight: #22313D;
  --enature-background-border: #2C3C49;

  --enature-text-primary: #E9EEF3;
  --enature-text-secondary: #B8C4CF;
  --enature-text-muted: #8C9DAB;

  --enature-accent-red: #F69B96;     /* Strictly quarantined for errors */
  --enature-accent-orange: #F3A76A;
  --enature-accent-yellow: #F2CC6B;  /* Low-fatigue cursor & highlights */
  --enature-accent-green: #8FD694;
  --enature-accent-teal: #56D6CA;
  --enature-accent-blue: #79BDFF;
  --enature-accent-purple: #C2A8F3;
  --enature-accent-pink: #F49BC1;

  --enature-body-weight: 450;
}

/* eNature Light (Daylight Proofreading Fallback) */
[data-theme='light'], [data-enature='light'] {
  --enature-background-base: #F5F7F9;
  --enature-background-raised: #ECEFF3;
  --enature-background-overlay: #E2E7EC;
  --enature-background-highlight: #D3DAE2;
  --enature-background-border: #BFC8D2;

  --enature-text-primary: #121A22;
  --enature-text-secondary: #3B4856;
  --enature-text-muted: #586776;

  --enature-accent-red: #A81E14;
  --enature-accent-orange: #8F4106;
  --enature-accent-yellow: #6E4C08;
  --enature-accent-green: #1A6B33;
  --enature-accent-teal: #0B6A63;
  --enature-accent-blue: #1A55A3;
  --enature-accent-purple: #5F3DB5;
  --enature-accent-pink: #A3244F;

  --enature-body-weight: 400;
}
```

The header theme toggle switches cleanly between **eNature Dark** and **eNature Light**. You can also press **`T`** anywhere on your keyboard to toggle.

---

## 7. Production Theme Downloads

Download ready-to-use configurations for your workflow:
- [**enature.json**](/assets/themes/enature.json) — Complete machine-readable eNature palette specification.
- [**enature.css**](/assets/themes/enature.css) — Standalone web CSS tokens and presets.
- [**enature-vscode.json**](/assets/themes/enature-vscode.json) — VS Code / Cursor editor theme.
- [**enature-alacritty.toml**](/assets/themes/enature-alacritty.toml) — Alacritty terminal configuration.
- [**enature.nvim**](/assets/themes/enature.nvim) — Standalone Neovim Lua colorscheme.
- [**choroid-v3.css**](/assets/themes/choroid-v3.css) — Legacy stylesheet with angular typography presets.
- [**choroid-v3.tokens.json**](/assets/themes/choroid-v3.tokens.json) — W3C Design Tokens format.
- [**choroid-v3-deep-color-theme.json**](/assets/themes/choroid-v3-deep-color-theme.json) — Legacy VS Code theme.
- [**choroid-v3-terminal.conf**](/assets/themes/choroid-v3-terminal.conf) — Ghostty and Kitty terminal configuration.
