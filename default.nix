{ lib
, llvmPackages
, cmake
, spdlog
, abseil-cpp }:

let
    discount_src = builtins.fetchGit{
        url = "https://github.com/Orc/discount.git";
        rev = "c214286f91cb754450121ad1f55a0e9470d3a16c";
    };
in llvmPackages.stdenv.mkDerivation rec {
    pname = "msg";
    version = "0.1";

    src = ./.;

    nativeBuildInputs = [ cmake ];

    cmakeFlags = [
        "-DDISCOUNT_SRC=${discount_src}"
    ];

    meta = with lib; {
        homepage = "https://github.com/compromyse/msg";
        description = "Minimal Static Site Generator";
        licencse = licenses.gpl3;
        platforms = with platforms; linux ++ darwin;
        maintainers = [ maintainers.compromyse ];
    };
}
