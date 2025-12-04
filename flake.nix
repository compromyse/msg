{
    inputs = {
        nixpkgs.url = "github:nixos/nixpkgs/nixos-unstable";
        self.submodules = true;
    };

    outputs = { self, nixpkgs, ... }:
    let
        pkgs = import nixpkgs { system = "x86_64-linux"; };
    in {
        devShells.x86_64-linux.default = pkgs.mkShell {
            buildInputs = with pkgs; [
                clang-tools
                    clang
                    lldb
                    ccls

                    cmake
                    ninja

                    valgrind
            ];
        };

        packages.x86_64-linux.default = pkgs.callPackage ./default.nix {};
    };
}
