# osm-live-updates

The [osm-live-updates](https://github.com/nicolano/osm-live-updates) (`olu`) tool is designed to 
keep SPARQL endpoints containing [*OpenStreetMap*](https://www.openstreetmap.org) (OSM) data that 
has been converted to RDF triples with [*osm2rdf*](https://github.com/ad-freiburg/osm2rdf), up to 
date by processing [*OsmChange*](https://wiki.openstreetmap.org/wiki/OsmChange) files. Since 
*osm2rdf* preserves the complete geometry of the OSM objects, `olu` also preserves the correctness 
of these geometries by updating the geometry of OSM objects in the database that reference a changed 
object in the *OsmChange* file.

## Preconditions

The OpenStreetMap (OSM) data must be converted to [RDF Turtle](https://www.w3.org/TR/turtle/) (TTL)
format using the osm2rdf tool, with the `--add-way-node-order` option enabled. To maintain data 
consistency and reduce the size of the resulting `.ttl` file, the `--write-ogc-geo-triples none` 
option can be used optionally, as updating GeoSPARQL (`ogc:`) triples is not currently supported.

## OsmChange files

You can use a folder containing one or more change files as input. If you use multiple change files, 
you have to make sure that they are lexicographically sorted by their filename in the correct order 
from oldest to newest.

You can also specify a server containing the change files as input, such as the one for the complete 
osm data, which can be found [here](https://planet.openstreetmap.org/replication/), for minutely, hourly and daily diffs. Subsets can be found 
at [Geofabrik](https://download.geofabrik.de), for example the daily diffs for [Germany](http://download.geofabrik.de/europe/germany-updates/).

## Getting started

### Docker

You can use the provided `Dockerfile` to compile `olu`:

```
docker build -t olu .
```

If you want to update a SPARQL endpoint from local change files, use the `-i` option to specify the 
directory where the files are located:

```
mkdir input
docker run --rm -it -v `pwd`/input/:/input/ olu SPARQL_ENDPOINT_URI -i /input
```

If you want to update a SPARQL endpoint that contains the complete OSM planet dataset from the OSM 
replication server, you can specify the URL of the server with the `-f` option:

```
docker run --rm -it olu SPARQL_ENDPOINT_URI -f https://planet.openstreetmap.org/replication/day/
```

By default, `olu` directly updates the SQL endpoint you specify. If you want to receive the SPARQL 
update queries as text-output instead, you can use the `-o` option:

```
mkdir output
docker run --rm -v `pwd`/output/:/output/ -it olu SAPRQL_ENDPOINT_URI -f https://planet.openstreetmap.org/replication/day/ -o /output/sparqlOutput.txt 
```

To see additional options, use the `--help` command.