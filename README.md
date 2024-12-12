# osm-live-updates

The osm-live-updates (`olu`) tool is designed to keep SPARQL databases containing 
[OpenStreetMap](https://www.openstreetmap.org) (OSM) data up to date by processing OsmChange files. It not only applies the 
changes to osm objects described in these files but also updates the geometries of objects affected 
by modifications to referenced elements, such as ways or relations.

## Preconditions

The OpenStreetMap (OSM) data must be converted to [RDF Turtle](https://www.w3.org/TR/turtle/) (TTL)
format using the osm2rdf tool, with the `--add-way-node-order` option enabled. To maintain data 
consistency and reduce the size of the resulting `.ttl` file, the `--write-ogc-geo-triples none` 
option can optionally be used, as the update of GeoSPARQL (`ogc:`) triples is currently not supported.

## OsmChane files

You can specify the path to a folder containing one or multiple change files with `-i`. If you use multiple change files you have to make sure that they can be sorted by their filename in 
the correct order from oldest to newest.

You can also specify a server containing the change files like the one for the complete osm data, which can be found
[here](https://planet.openstreetmap.org/replication/), for minutely, hourly and daily diffs. 
Subsets can be found at [Geofabrik](https://download.geofabrik.de), for example the daily diffs 
for [Germany](http://download.geofabrik.de/europe/germany-updates/).

## Getting started

### Docker

You can use the provided `Dockerfile` to compile `olu`:

```
docker build -t olu .
```

If you want to updated a SPARQL endpoint that contains the complete OSM planet dataset, for example,
you can run `olu` with:

```
docker run --rm -it olu apps/olu -u `sparql-endpoint-url` -d `https://planet.openstreetmap.org/replication/day/`
```

