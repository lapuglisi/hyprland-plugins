{
  lib,
  hyprland,
  hyprlandPlugins,
}:
hyprlandPlugins.mkHyprlandPlugin {
  pluginName = "hyprland-tabbed";
  version = "0.1";
  src = ./.;

  inherit (hyprland) nativeBuildInputs;

  meta = with lib; {
    homepage = "https://github.com/lapuglisi/hyprland/tree/main/hyprland-tabbed";
    description = "Hyprland tabbed layout sway-like plugin";
    license = licenses.bsd3;
    platforms = platforms.linux;
  };
}