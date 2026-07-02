#ifndef THEME_H
#define THEME_H

#include <QString>
#include <QApplication>

// ══════════════════════════════════════════════
//  医慧通 · 数字疗愈 Design System — Qt Edition
// ══════════════════════════════════════════════

namespace Theme {

// ── Brand Palette ──
inline constexpr auto Brand       = "#0E6B6E";
inline constexpr auto BrandDark   = "#094D4F";
inline constexpr auto BrandLight  = "#E8F4F4";
inline constexpr auto BrandHover  = "#0B5658";

// ── Warm Accent ──
inline constexpr auto Warm        = "#DC6B3E";
inline constexpr auto WarmLight   = "#FDF0EA";
inline constexpr auto WarmBorder  = "#FBE1D5";

// ── Surface ──
inline constexpr auto Surface     = "#FAFAF8";
inline constexpr auto SurfaceAlt  = "#F5F4F0";
inline constexpr auto White       = "#FFFFFF";

// ── Text ──
inline constexpr auto Ink         = "#1C1917";
inline constexpr auto InkMuted    = "#78716C";

// ── Semantic ──
inline constexpr auto Success     = "#059669";
inline constexpr auto Error       = "#E11D48";
inline constexpr auto Info        = "#3B82F6";
inline constexpr auto Warning     = "#F59E0B";

// ── Border ──
inline constexpr auto Border      = "rgba(0,0,0,0.05)";
inline constexpr auto BorderDark  = "rgba(0,0,0,0.08)";

// ── Radius ──
inline constexpr int  RadiusXs    = 4;
inline constexpr int  RadiusSm    = 8;
inline constexpr int  RadiusMd    = 12;
inline constexpr int  RadiusLg    = 16;
inline constexpr int  RadiusXl    = 20;
inline constexpr int  Radius2xl   = 24;
inline constexpr int  Radius3xl   = 32;

// ── Font ──
inline constexpr auto FontFamily  = "\"Microsoft YaHei\", \"PingFang SC\", -apple-system, sans-serif";
inline constexpr int  FontXs      = 10;
inline constexpr int  FontSm      = 12;
inline constexpr int  FontMd      = 14;
inline constexpr int  FontLg      = 18;
inline constexpr int  FontXl      = 22;
inline constexpr int  Font2xl     = 28;

// ── Spacing ──
inline constexpr int  SpaceXs     = 4;
inline constexpr int  SpaceSm     = 8;
inline constexpr int  SpaceMd     = 12;
inline constexpr int  SpaceLg     = 16;
inline constexpr int  SpaceXl     = 24;

// ── Build & Apply Global Stylesheet ──
QString globalStyleSheet();

void applyTheme(QApplication &app);

} // namespace Theme

#endif // THEME_H
