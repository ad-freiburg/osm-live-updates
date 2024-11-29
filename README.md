# osm-live-updates

osm-live-updates (`olu`)  is a tool that can be used to keep databases with 
[OpenStreetMap](https://www.openstreetmap.org) (OSM) data that are managed by the [QLever](https://github.com/ad-freiburg/qlever) SPARQL engine up to date by 
using [OsmChange](https://wiki.openstreetmap.org/wiki/OsmChange) files.

The [osm2rdf](https://github.com/ad-freiburg/osm2rdf) tool is used to convert the osm data to [RDF Turtle](https://www.w3.org/TR/turtle/) (TTL). 

## Accompanying services and materials

The directory containing the osm change files for the complete osm data can be found
[here](https://planet.openstreetmap.org/replication/), for minutely, hourly and daily diffs. 
Subsets can be found at [Geofabrik](https://download.geofabrik.de), for example the daily diffs 
for [Germany](http://download.geofabrik.de/europe/germany-updates/).

## Getting started

### Docker

You can use the provided `Dockerfile` to compile `olu`:

```
docker build -t olu .
```

You can then run `olu` when you want to update a SPARQL endpoint with the daily osm-planet diffs, for example:

```
docker run --rm -it olu apps/olu -u http://host.docker.internal:7007/osm-planet/ -d https://planet.openstreetmap.org/replication/day/
```

