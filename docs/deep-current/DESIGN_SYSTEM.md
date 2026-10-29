# Wolf Audio Tide — Deep Current visual design system

## Identity
Preserve Tide's two equal-status panels and original positioning. Left DELAY: warm graphite satin metal. Right REVERB: deep desaturated turquoise. Surrounding shell: navy-black satin titanium. Provide original Wolf Audio identity, not a hardware replica of Strymon TimeLine or BigSky.

## Palette (targets, subject to screenshot review)
- Chassis `#19272E`
- Delay `#5B6568`
- Reverb `#246E76`
- Glass `#0C222A`
- Teal active accent `#77D7D5`
- Ivory text `#F4EADC`

## Typography
Keep the Tide logo as a separate brand asset. Use one modern, appropriately licensed sans (Inter or IBM Plex Sans suggested) for all interface labels, values, buttons and menus. Readable at 900px, 1080px, 1620px and Retina scaling; consistent baselines, optical sizing, weights, capitalization and tabular numerals. Never bake parameter text or numeric values into an image.

## Main/secondary knobs
Use `Tide_05` and `Tide_06` as *source art*, not final sprites. Smoked titanium, thin machined bevel, soft upper-left specular reflection; static cached knob cap, dynamic vector value arc and pointer; consistent visual grammar on every page. Preserve existing slider attachments, unit formatting, drag, fine control, double click, automation and numeric editing. Do not rotate a lit bitmap if highlights become physically unnatural.

## Panels/screens
`Tide_01–04`: static light metal textures and separate recessed sections, understated texture, no high-frequency noisy detail. Screens `Tide_07–09`: two coordinated OLED bezels. Actual stereo delay tap lanes, feedback motion, rhythmic dots, reverb wave curves, activity and dynamic information remain JUCE-drawn and driven by the existing state. Glass overlay is optional; drop if it reduces clarity.

## Buttons/selectors/sliders
`Tide_10–12`: contact sheets are design references. Implement scale-safe JUCE vector or nine-slice surfaces with visible neutral/hover/pressed/active/locked/disabled states. Preserve existing hit boxes and exact event behavior. Live text, arrows, slider fill and status remain programmatic.

## Icons/branding
`Tide_13–14`: references only. Rebuild clean scalable icons and optional understated wave accents; no automatically accepted AI-generated symbol geometry.

## All views
Main editor, advanced Delay and Reverb, independent machine pages, preset browser, popup menus and global controls must share the same components. Keep UI readable and accessible. Prioritize tasteful depth over decorative machinery or excessive glow.
