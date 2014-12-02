#!/bin/bash

DATA_DIR=`pwd`
SOURCES_DIR=$DATA_DIR/translations/sources
ADDITIONAL_DIR=$DATA_DIR/translations/additional
OUTPUT_DIR=$DATA_DIR/translations/output

cd $SOURCES_DIR
for i in *.yml;
do
  if [ -f $ADDITIONAL_DIR/$i ];
  then
    echo "Additional translation exists for file $i, merging..."
    cat $SOURCES_DIR/$i $ADDITIONAL_DIR/$i > $OUTPUT_DIR/$i
  else
    echo "No additional translations exist for file $i, copying..."
    cp $SOURCES_DIR/$i $OUTPUT_DIR/$i
  fi
done

cd $OUTPUT_DIR
zip latest.zip *
mv latest.zip $DATA_DIR/
