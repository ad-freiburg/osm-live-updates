# osm-live-updates

The [osm-live-updates](https://github.com/nicolano/osm-live-updates) (`olu`) tool is designed to 
keep SPARQL endpoints containing [*OpenStreetMap*](https://www.openstreetmap.org) (OSM) data, which 
has been converted to RDF triples with [*osm2rdf*](https://github.com/ad-freiburg/osm2rdf), up to 
date by processing [*OsmChange*](https://wiki.openstreetmap.org/wiki/OsmChange) files. Since 
*osm2rdf* retains the complete object geometry of the OSM data, `olu` also preserves the correctness 
of these geometries by updating the geometry of OSM objects in the database that reference a changed 
object in the *OsmChange* file.

## Preconditions

The OpenStreetMap (OSM) data must be converted to [RDF Turtle](https://www.w3.org/TR/turtle/) (TTL)
format using the osm2rdf tool, with the `--add-way-node-order` option enabled. To maintain data 
consistency and reduce the size of the resulting `.ttl` file, the `--write-ogc-geo-triples none` 
option can optionally be used, as the update of GeoSPARQL (`ogc:`) triples is currently not supported.

## OsmChane files

You can a folder containing one or multiple change files as input. If you use multiple change files 
you have to make sure that they are lexicographically sorted by their filename in the correct order 
from oldest to newest one.

You can also specify a server containing the change files as input, such as the one for the complete 
osm data, which can be found [here](https://planet.openstreetmap.org/replication/), for minutely, hourly and daily diffs. Subsets can be found 
at [Geofabrik](https://download.geofabrik.de), for example the daily diffs for [Germany](http://download.geofabrik.de/europe/germany-updates/).

## Getting started

### Docker

You can use the provided `Dockerfile` to compile `olu`:

```
docker build -t olu .
```

If you want to update a SPARQL endpoint from local files you can run `olu` with:

```
mkdir input
docker run --rm -it -v `pwd`/input/:/input/ olu SAPRQL_ENDPOINT_URI -i /input
```

If you want to updated a SPARQL endpoint that contains the complete OSM planet dataset from the osm 
replication server, you can run `olu` with:

```
docker run --rm -it olu SAPRQL_ENDPOINT_URI -f https://planet.openstreetmap.org/replication/day/
```

By default, `olu` directly updates the SQL endpoint you specify. If you want to receive the sparql 
update queries as text-output instead, do:

```
mkdir input
mkdir output
docker run --rm -v `pwd`/input/:/input/ -v `pwd`/output/:/output/ -it olu SAPRQL_ENDPOINT_URI -i /input -o /output/sparqlOutput.txt 
```