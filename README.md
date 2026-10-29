# Untwine

Untwine is software from [Hobu, Inc.](https://hobu.co) for creating [Cloud Optimized Point Cloud](https://copc.io/) (COPC) web services from [PDAL](https://pdal.io)-readable point cloud data sources.


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
untwine -i some_directory_of_input_files -o output_filename
```

or

```
untwine --files=some_directory_of_input_files --output_file=output_filename
```


Options
-------

- `-i` or `--files`

  Input files or directories containing input files. [Required]

- `-o` or `--output_dir`

  Output file. [Required]

- `--help`

  Usage help

- `--a_srs`

  Assign an output SRS. Example `--a_srs EPSG:2056`

- `--no_srs`

  Don't read the SLR VLRs. This is only applicable to las files. [Default: false]

- `--dims`

  List of dimensions to load. X, Y and Z are always loaded. Limiting the dimensions can
  speed runtime and reduce temporary disk use.

- `--temp_dir`

  Directory in which to place tiled output. If not provided, temporary files are placed
  in 'output_dir'/temp.

- `--cube`

  Create a voxel structure where each voxel is a cube. If false, the voxel structure is
  a rectangular solid that encloses the points. [Default: true]

- `--level`

  Level to use when initially tiling points.  If not provided, an initial level is
  determined from the data. [Default: none].

- `--file_limit`

  Only read 'file_limit' input files even if more exist in the 'files' list. Used primarily
  for debugging. [Default: no limit]

- `--threads`

  Number of worker threads used for tiling and sampling. Memory use scales with the thread
  count. [Default: 0, meaning the number of hardware threads]

- `--buffer_size`

  Capacity of each [staging buffer](#staging-buffers) in bytes. Must
  be at least as large as the expanded data for a single point.  [Default: 40960]

- `--buffer_count`

  Maximum number of [staging buffers](#staging-buffers). Must exceed the
  resolved worker thread count. (see[Default: 1000]

- `--stats`

  Generate summary statistics in 'ept.json' similar to those produced by Entwine for EPT output
  Min/max stats are always generated when generating single-file output.
  [Default: false]

- `--progress_fd`

  File descriptor number of a pipe using the Untwine API to send progress and error messages.
  [Default: -1]

- `--progress_debug`

  Set to true to have progress messages written to standard output. Disabled if 'progress_fd'
  is set to '1'. [Default: false]

- `-s` or `-- single_file`

  **Deprecated and ingored.** Generate a LAZ file with spatially arranged data and hierarchy information
  [(COPC)](https://github.com/copcio/copcio.github.io). [Default: false]

### Staging buffers

Staging buffers hold points belonging to a specific voxel before being
written to a temporary data file. When a buffer becomes full, it is written
to a temporary output file. Larger buffers can reduce
the number of data writes. More buffers can allow data from a larger number
of voxels to be stored before a write is required.  Increasing buffer size or
count increases memory use.

