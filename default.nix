{ lib
, llvmPackages
, cmake
, spdlog
, abseil-cpp }:

llvmPackages.stdenv.mkDerivation rec {
  pname = "msg";
  version = "0.1";

  src = ./.;

  nativeBuildInputs = [ cmake ];

  meta = with lib; {
    homepage = "https://github.com/compromyse/msg";
    description = "Minimal Static Site Generator";
    licencse = licenses.gpl2;
    platforms = with platforms; linux ++ darwin;
    maintainers = [ maintainers.compromyse ];
  };
}
