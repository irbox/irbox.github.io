---
title: Theming
description: Nature (Choroid v4.0) color system, 3-variant architecture, viewing distance calibration, and clinical caveats.
category: Customizing
---

**Nature** (incorporating the **Choroid v4.0** optical architecture) is the core theme system for this site. Grounded in retinal neurobiology and optical coherence tomography (OCT) research (*Aleman, Wang, & Schaeffel 2018, Scientific Reports 8:10840*; *Swiatczak & Schaeffel 2022, TVST*; *Ingrassia et al. 2024, Vision Res*), the design architecture features three rigorously calibrated variants:

- **Nature Deep (`data-theme='dark'` / `data-choroid='deep'`)**: The primary biological baseline. Maximum contrast (16.5:1), deep ocular substrate (`#0A0F14`) activating the retinal ON-pathway through positive contrast polarity, promoting choroidal thickening (+10 µm) and inhibiting scleral axial elongation.
- **Nature Soft (`data-theme='soft'` / `data-choroid='soft'`)**: Astigmatism- and glare-friendly dark mode. Calibrated 13.6:1 contrast (`#121A22` base, `#DCE3EA` text) engineered to minimize optical halation, edge fuzzing, and uncorrected astigmatic blur while maintaining ON-pathway dominance.
- **Nature Lumen (`data-theme='light'` / `data-choroid='lumen'`)**: Daylight and high-ambient fallback (16.3:1 contrast, `#F5F7F9` base, `#121A22` text). Engineered for proofreading, high-glare window workstations, and older readers. **Explicitly documented as NOT myopia-informed** (inverted contrast is not present).

---

## Calibrated Variants Comparison

| Metric / Attribute | Deep (Baseline) | Soft (Reduced Glare) | Lumen (Daylight Fallback) |
| :--- | :--- | :--- | :--- |
| **Role** | Myopia-informed primary baseline | Astigmatism-friendly dark mode | Daylight / proofreading fallback |
| **Contrast Polarity** | Positive (Light on dark) | Positive (Light on dark) | Negative (Dark on light) |
| **Base Background** | `#0A0F14` | `#121A22` | `#F5F7F9` |
| **Raised Surface** | `#101820` | `#18222C` | `#ECEFF3` |
| **Overlay Card** | `#17222C` | `#1F2B36` | `#E2E7EC` |
| **Highlight Border** | `#2C3C49` | `#364857` | `#BFC8D2` |
| **Primary Text** | `#E9EEF3` (16.5:1) | `#DCE3EA` (13.6:1) | `#121A22` (16.3:1) |
| **Secondary Text** | `#B4C0CB` (9.4:1) | `#AEBBC7` (8.8:1) | `#3B4856` (9.2:1) |
| **Muted Caption** | `#74889A` (5.2:1) | `#8497A8` (5.4:1) | `#5F6E7D` (5.1:1) |
| **Myopia-Informed?** | **Yes** (Aleman et al. 2018) | **Yes** (Aleman et al. 2018) | **No** (Standard contrast) |

---

## Reading-Size Calibration: The ~34 Arcmin Rule

A crucial finding from follow-up research (*Swiatczak & Schaeffel 2022, TVST*; *Schaeffel 2025 ZEISS Young Investigator lecture*) is that **inverted contrast alone is insufficient if the text is too small**:

> In the 2022 clinical trials, only large text (capital letter height $\sim 34.4\text{ arcmin}$) produced statistically significant axial eye shortening. Small text ($\sim 13.75\text{ arcmin}$, typical of unmagnified mobile phone screens) produced **no statistically significant effect** ($p = 0.09$).

To trigger the protective retinal ON-pathway response, typography must subtend approximately $34\text{ arcmin}$ of visual angle at your actual viewing distance:

| Viewing Distance | Visual Angle | CSS Font Size | Recommended Device / Context |
| :---: | :---: | :---: | :--- |
| **30 cm** | $\sim 34\text{ arcmin}$ | `16px` | Minimum handheld distance (close smartphone) |
| **35 cm** | $\sim 34\text{ arcmin}$ | `19px` | Relaxed handheld smartphone reading |
| **40 cm** | $\sim 34\text{ arcmin}$ | `22px` | **Preferred reading distance** (tablets, small laptops) |
| **50 cm** | $\sim 34\text{ arcmin}$ | `27px` | Standard laptop distance on desk |
| **60 cm** | $\sim 34\text{ arcmin}$ | `32px` | Standard external desktop monitor (24″–27″) |
| **70 cm** | $\sim 34\text{ arcmin}$ | `38px` | Deep desktop monitor or large workstation screen |

You can switch viewing distance presets on any documentation or blog page using the calibrated distance toolbar above the article header. Your selection is automatically persisted in `localStorage`.

---

## Neurobiological Spectrum Rules

### 1. Full-Spectrum Neutral Body Text (No Red-Only Filters)

Many popular night software tools apply heavy amber or red filters that strip all blue and green wavelengths. While useful for circadian melatonin preservation right before sleep, **red-only reading filters are counterproductive for myopia control**:

- *Ingrassia, Swiatczak, & Schaeffel (2024, Vision Research)* tested red-in-focus reading filters and discovered that **red-filtered reading paradoxically triggered axial eye lengthening** in emmetropic subjects.
- By eliminating short-wavelength light, red filters starve the retina of high-frequency blue-channel contrast and push hyperopic retinal blur signals backward behind the retina.
- Nature maintains **neutral, full-spectrum balanced body text** (`#E9EEF3` / `#DCE3EA`) across all reading surfaces.

### 2. Red Strictly Reserved for Errors

Under dark mode conditions, high-saturation red text causes severe visual fatigue and accommodative micro-fluctuations (*Sensors 2024* study on visual fatigue in dark-mode displays):

- Red text under dark mode was clinically documented to produce the **highest visual fatigue**, whereas yellow and teal produced the least.
- Nature strictly reserves red (`#F2736D` in Deep, `#F4817B` in Soft, `#A81E14` in Lumen) for compiler diagnostics, exceptions, and critical warnings.
- High-frequency syntactic tags and attributes use pink (`#F28BB8` / `#EE90B8` / `#A3244F`) and amber-orange (`#F29E5C` / `#EEA062` / `#8F4106`).

---

## Screen-Time & Pseudomyopia Protocol

Grounded in International Myopia Institute (IMI 2023, 2025) consensus and clinical pediatric trials:

| Parameter | Recommended Protocol | Scientific / Clinical Basis |
| :--- | :--- | :--- |
| **Viewing Distance** | $\ge 30\text{ cm}$ ($\mathbf{40\text{ cm}}$ preferred) | Close reading under 30 cm is an independent clinical risk factor for myopia progression. |
| **Continuous Reading** | $\le 30\text{ minutes}$ | Continuous near work over 30–45 min promotes ciliary spasm and accommodative lag. |
| **Distance Break** | $\ge 5\text{ minutes}$ | Relax accommodation by focusing on distant outdoor targets with fine spatial detail. |
| **Ambient Room Light** | $500\text{ to }1000\text{ lux}$ | **Never read a dark theme in a pitch-black room.** 500–1000 lux ambient lighting thickens the choroid. |
| **Outdoor Sunlight** | $\sim 2\text{ hours / day}$ | The single most clinically validated protective intervention for children and young adults (IMI 2025). |
| **Display Brightness** | Moderate ($35\text{–}85\text{ cd/m}^2$) | Matches the illuminance levels proven in the original Aleman (2018) OCT trials. |
| **Children Under 3** | Zero screen time | WHO, AAP, and China MOE (2025) pediatric guidelines. |

---

## Age-Specific Ergonomic Guidance

| Age Group | Clinical Priority & Recommendations | Variant Selection |
| :--- | :--- | :--- |
| **Children (Ages 3–12)** | Outdoor time ($\sim 2\text{ h/day}$), viewing distance $\ge 30\text{ cm}$, and regular distance breaks. Dark mode benefit is unproven in children. | Deep or Soft with large text (`19px`–`22px`) as an optional supplement. |
| **Teens & Young Adults (Ages 13–25)** | **Highest-risk group for adult-onset progression** (students, software engineers). Distance breaks, outdoor time, enlarged text. | **Nature Deep** or **Soft** baseline. |
| **Adults (Ages 25–40)** | Myopia progression typically stabilizes ($< 0.10\text{ D/yr}$). Benefit of inverted contrast is uncertain; select by visual comfort. | **Nature Soft** (low glare) or **Lumen**. |
| **Adults (Ages 40–60)** | Accommodative amplitude diminishes (presbyopia); check optical correction for astigmatism first. | **Nature Soft** or **Lumen** (high legibility). |
| **Adults (60+ Years)** | Lens scatter, pupil miosis, cataract or AMD can invert contrast preferences. Choose purely by legibility. | **Nature Soft** or **Lumen** based on personal comfort. |

---

## Clinical Caveats & Pseudomyopia Guidance

> [!IMPORTANT]
> **Design Rationale, Not Medical Treatment:**
> The Nature / Choroid color palette and viewing distance presets are ergonomic design tools based on short-term laboratory optical coherence tomography (OCT) findings. They are not medical treatments, do not cure refractive errors, and are not guaranteed to prevent myopia.

> [!WARNING]
> **Accommodative Spasm ("Pseudomyopia"):**
> If your distance vision becomes blurry following prolonged computer work and recovers over minutes to hours, or if near work is accompanied by frontal headaches and eyestrain, you may be experiencing **ciliary accommodative spasm (pseudomyopia)**.
> 
> A color palette cannot treat or release accommodative spasm. Pseudomyopia requires a comprehensive eye examination by an optometrist or ophthalmologist, typically including **cycloplegic refraction** (eye drops that temporarily relax the ciliary muscle).

---

## Design Tokens

Tokens are standard CSS custom properties declared at the root of `assets/css/main.css`:

```css
/* Deep Variant (Baseline) */
:root, [data-theme='dark'], [data-choroid='deep'] {
  --bg-body: #0A0F14;
  --bg-surface: #101820;
  --bg-surface-raised: #17222C;
  --bg-surface-elevated: #22313D;
  --border: #2C3C49;
  --ink: #E9EEF3;
  --ink-2: #B4C0CB;
  --ink-muted: #74889A;
  --brand: #4FD1C5;
  --danger: #F2736D;
}

/* Soft Variant (Low Glare / Astigmatism) */
[data-theme='soft'], [data-choroid='soft'] {
  --bg-body: #121A22;
  --bg-surface: #18222C;
  --bg-surface-raised: #1F2B36;
  --bg-surface-elevated: #2A3946;
  --border: #364857;
  --ink: #DCE3EA;
  --ink-2: #AEBBC7;
  --ink-muted: #8497A8;
  --brand: #55CBC0;
  --danger: #F4817B;
}

/* Lumen Variant (Daylight Fallback) */
[data-theme='light'], [data-choroid='lumen'] {
  --bg-body: #F5F7F9;
  --bg-surface: #ECEFF3;
  --bg-surface-raised: #E2E7EC;
  --bg-surface-elevated: #D3DAE2;
  --border: #BFC8D2;
  --ink: #121A22;
  --ink-2: #3B4856;
  --ink-muted: #5F6E7D;
  --brand: #0B6A63;
  --danger: #A81E14;
}
```

The topbar theme toggle cycles through **Deep &rarr; Soft &rarr; Lumen &rarr; Deep**. You can also press **`T`** anywhere on the keyboard to cycle through the variants.

---

## Editor & Terminal Palette Exports

Download ready-to-use configurations for your workflow:
- [Visual Studio Code Theme (`assets/themes/nature-vscode.json`)](/assets/themes/nature-vscode.json)
- [Neovim Lua Colorscheme (`assets/themes/nature.nvim`)](/assets/themes/nature.nvim)
- [Alacritty Terminal Configuration (`assets/themes/nature-alacritty.toml`)](/assets/themes/nature-alacritty.toml)
- [Standalone Web CSS Tokens (`assets/themes/nature.css`)](/assets/themes/nature.css)
- [Machine-Readable Palette Specification (`assets/themes/nature.json`)](/assets/themes/nature.json)
- [Reference Choroid v4.0 Specification (`assets/themes/choroid.json`)](/assets/themes/choroid.json)
- [Reference Choroid CSS (`assets/themes/choroid.css`)](/assets/themes/choroid.css)
