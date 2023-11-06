# Untwine

Untwine is software from [Hobu, Inc.](https://hobu.co) for creating [Entwine Point Tile](https://entwine.io/entwine-point-tile.html)  (EPT) 
or [Cloud Optimized Point Cloud](https://copc.io/) (COPC) web services from [PDAL](https://pdal.io)-readable point cloud data sources. It
provides an alternative processing approach than the [Entwine](https://entwine.io)
software, but the output is expected to be compatible EPT/COPC.


License
-------

Untwine is licensed under the GPLv3. Commercial licensing is possible by contacting
Hobu, Inc. for pricing.

Installation
------------

Untwine is available on `conda-forge` ([here](https://anaconda.org/conda-forge/untwine)) and can be installed using `conda`

```
conda install -c conda-forge untwine
```


Building Untwine:
-----------------

The following steps will build the `untwine` executable:
```
mkdir build
cd build
cmake ..
make
```

Using Untwine
-------------

```
untwine [options]
```

Example:
--------

```
untwine --files=some_directory --output_dir=output_directory
```

Options
-------

- files

  Input files or directories containing input files. [Required]

- output_dir

  Output directory. [Required]

- a_srs

  Assign an output SRS. Example `--a_srs EPSG:2056`
  
- dims

  List of dimensions to load. X, Y and Z are always loaded. Limiting the dimensions can
  speed runtime and reduce temporary disk use.

- temp_dir

  Directory in which to place tiled output. If not provided, temporary files are placed
  in 'output_dir'/temp.

- cube

  Create a voxel structure where each voxel is a cube. If false, the voxel structure is
  a rectangular solid that encloses the points. [Default: true]

- level

  Level to use when initially tiling points.  If not provided, an initial level is
  determined from the data. [Default: none].

- file_limit

  Only read 'file_limit' input files even if more exist in the 'files' list. Used primarily
  for debugging. [Default: no limit]

- stats

  Generate summary statistics in 'ept.json' similar to those produced by Entwine for EPT output
  Min/max stats are always generated when generating single-file output.
  [Default: false]

- single_file

  Generate a LAZ file with spatially arranged data and hierarchy information
  [(COPC)](https://github.com/copcio/copcio.github.io). [Default: false]

- preserve_temp_dir

  Normally untwine deletes its temporary directory where tiled data is written.  Set this
  to true to preserve an existing temporary directory and its contents. [Default: false]

- progress_fd

  File descriptor number of a pipe using the Untwine API to send progress and error messages.
  [Default: -1]

- progress_debug

  Set to true to have progress messages written to standard output. Disabled if 'progress_fd'
  is set to '1'. [Default: false]
