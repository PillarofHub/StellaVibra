# Stella Vibra 브랜드 이미지

- `stella-vibra.png`: 사용자가 승인한 앱 아이콘 원본. 노란 네 갈래 별, 하늘색 진동선, 남색 배경.
- `stella-vibra.ico`: 원본을 `scripts/make-icon.ps1`로 변환한 Windows 다중 해상도 아이콘.
- `stella-vibra-logo.png`: README용 가로형 프로젝트 로고. Stella는 별의 노란색, Vibra는 진동선의 하늘색. 남색 배경을 포함해 밝은/어두운 README 테마에서 동일하게 표시한다.

가로형 로고는 내장 image_gen 도구로 아이콘 원본을 참조하여 생성했다. 앱 아이콘을 대체하지 않으며 README와 배포 폴더의 README에 사용한다. 생성 후 추가 이미지 편집은 하지 않았다.

## 가로형 로고 생성 프롬프트

```text
Use case: logo-brand. Make a finished wide horizontal project logo banner for Stella Vibra, approximately 3:1 aspect ratio, intended for a GitHub README header. Reference image is the approved brand symbol, preserve its identity: plump symmetric four-point golden-yellow star between exactly two thick sky-blue parenthesis-like vibration strokes. Place this same emblem on the left of the banner at moderate size, and the exact wordmark 'Stella Vibra' to its right on ONE line, vertically centered alongside it. Stella must be the same golden yellow as the star, Vibra must be the same sky blue as the vibration strokes. Use beautiful clean bold modern sans-serif lettering, balanced kerning, clear ordinary letterforms, capital S and capital V, all other letters lowercase. Solid deep midnight navy background matching the reference, full rectangular canvas. Integrate emblem directly into the banner background, no separate outlined tile. Generous but not excessive outer padding, balanced horizontal layout, emblem about the height of capital letters times 1.6, large easily legible wordmark. Flat professional design, no tagline, no additional text, no version number, no extra stars, no constellation, no glow, no shadow, no bevel, no 3D, no texture, no presentation mockup. Only the finished horizontal logo.
```
