<!--USE THIS TEMPLATE TO COMPLETE THE CHANGELOG-->
<!--
## [Version number] - YYYY-MM-DD
### Added
-

### Changed
-

### Deprecated
-

### Removed
-

### Fixed
-

### Security
-
-->

# Changelog

All notable changes to this project will be documented in this file.

## [Unreleased]

### Added

- CMakefile checks consistency of version number across git/c++/CMake
- Test for warnings coming from `include/htool/*`
  
### Changed

- Remove unnecessary arguments in HMatrix interface

## [0.6] - 2021-08-03

### Changed

- Python interface with ctypes deprecated, see new python [interface](https://github.com/htool-ddm/htool_python) with pybind11.
- Performance tests removed, see dedicated [repository](https://github.com/PierreMarchand20/htool_benchmarks).
- GUI deprecated.

## [0.5] - 2021-08-03

### Added

- Interface for clustering via `VirtualCluster`, implementation with `Cluster` whose template parameter is the type of clustering (supported now: `PCARegularClustering`, `PCAGeometricClustering`, `BoundingBox1RegularClustering` and `BoundingBoxGeometricClustering`).
- Interface for hmatrix objects via `VirtualHMatrix`, implementation with HMatrix whose template parameter is the type of compressor (supported now: `SVD`, `fullACA`, `partialACA`, `sympartialACA`, all derived from abstract class `LowRankMatrix`).
- Interface for generator via `VirtualGenerator`. The user needs to define an object deriving from `VirtualGenerator` to populate a hmatrix.
- `Block` class with different admissible condition used in `Block` (only `RjasanowSteinbach` at the moment).
- Interface for a sub-block via `IMatrix`, from which both `SubMatrix` and `LowRankMatrix` inherit. They respectively correspond to dense and compressed sub-blocks.
- `DDM` solvers via HPDDM, and coarse space building.
- Test suite and CI with GitHub action.