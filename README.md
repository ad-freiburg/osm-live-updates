# osm-live-updates

[osm-live-updates](https://github.com/nicolano/osm-live-updates) (`olu`) 
is a tool that generates SPARQL update operations from OSM change files,
enabling efficient and near real-time querying of OSM data on SPARQL endpoints.
When paired with a high-performance SPARQL engine like [QLever](https://github.com/ad-freiburg/qlever), 
`olu` can generate update operations from minute-interval change files 
for the complete OSM Planet dataset in under 7 seconds,
while ensuring the geometries of all OSM elements on the SPARQL endpoint remain correct.

## Preconditions

The OSM data on the SPARQL endpoint must be converted to [RDF Turtle](https://www.w3.org/TR/turtle/) format 
using the [`osm2rdf`](https://github.com/ad-freiburg/osm2rdf) tool.
To ensure data consistency and reduce the overall dataset size, 
use the `--write-ogc-geo-triples none` option during conversion. 
This disables the generation of spatial relationship triples, 
which are not supported by `olu`.

## Getting Started

### QLever

The easiest way to use olu is together with a [QLever](https://github.com/ad-freiburg/qlever) endpoint.
When the `qlever` command-line interface is installed, 
you can update a running QLever endpoint directly using:

```
qlever update-osm
```

This command internally uses `olu` to fetch and process the latest OSM change files for your SPARQL endpoint.
If you are working with a regional OSM extract, you can use either the `--bbox` or
`--polygon` option to limit the updates to that region. 
Using the `--granularity` option, 
you can also control the frequency of updates (e.g., each `minute`, `hour`, or `day`).

### Docker

You can use the provided `Dockerfile` to build `olu`:

```
docker build -t olu .
```

To run `olu`, provide the URI of the SPARQL endpoint you want to update as the first argument.
You can either update the endpoint using an OSM replication server or local change files.

Specify the replication server URL with the `-r` option:

```
docker run --rm -it olu http://localhost:7025 -r https://planet.openstreetmap.org/replication/day/
```

`olu` will automatically fetch and apply the latest change files from the specified replication server.

If you have local OSM change files, use the -i option to specify their directory:

```
mkdir input
docker run --rm -it -v `pwd`/input/:/input/ olu http://localhost:7025 -i /input
```

To see additional options, use the `--help` command.