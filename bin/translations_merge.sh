#!/bin/bash

# This line is taken from https://stackoverflow.com/questions/59895/can-a-bash-script-tell-what-directory-its-stored-in
DATA_DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
PROJECT=x-com-1
SOURCES_DIR=$DATA_DIR/translations/$PROJECT/sources
ADDITIONAL_DIR=$DATA_DIR/translations/$PROJECT/additional
OUTPUT_DIR=$DATA_DIR/translations/$PROJECT/output

for i in $SOURCES_DIR/*.yml;
do
  file=$(basename $i)
  if [ -f $ADDITIONAL_DIR/$file ];
  then
    echo "Additional translation exists for file $file, merging..."
    cat $SOURCES_DIR/$file $ADDITIONAL_DIR/$file > $OUTPUT_DIR/$file
  else
    echo "No additional translations exist for file $file, copying..."
    cp $SOURCES_DIR/$file $OUTPUT_DIR/$file
  fi
done
