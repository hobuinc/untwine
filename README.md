# Untwine

Untwine is software from [Hobu, Inc.](https://hobu.co) for creating [Entwine Point Tile](https://entwine.io/entwine-point-tile.html)  (EPT)
web services from [PDAL](https://pdal.io)-readable point cloud data sources. It
provides an alternative processing approach than the [Entwine](https://entwine.io)
software, but the output is expected to be compatible EPT.


## License

Untwine is licensed under the GPLv3. Commercial licensing is possible by contacting Hobu, Inc. for pricing.

## Using Untwine

Creation of EPT dataset is a two-step process:

1. run `epf` tool to process the input dataset and create buckets of points:
   ```
   epf --files my_file.laz --output_dir /tmp/epf-output
   ```

2. run `bu` tool to run the bottom-up indexing and produce the final EPT dataset:
   ```
   bu --input_dir /tmp/epf-output --output_dir /tmp/ept
   ```
