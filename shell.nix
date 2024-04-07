let
  pkgs = import <nixpkgs> {};
in
  pkgs.mkShell {
    packages = [
      pkgs.python3
      pkgs.tio
      pkgs.usbutils
      pkgs.pulseview
    ];
  }
