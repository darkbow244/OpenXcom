#!/bin/bash

# This line is taken from https://stackoverflow.com/questions/59895/can-a-bash-script-tell-what-directory-its-stored-in
DATA_DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
# So far, only the common elements have system-specific translations
PROJECT=common

TRANSLATIONS_SOURCE=$DATA_DIR/translations/sources
TRANSLATIONS_ADDITIONAL=$DATA_DIR/translations/additional
TRANSLATIONS_OUTPUT=$DATA_DIR/translations/output

SOURCES_DIR=$TRANSLATIONS_SOURCE/$PROJECT/Language
ADDITIONAL_DIR=$TRANSLATIONS_ADDITIONAL/$PROJECT/Language
OUTPUT_DIR=$TRANSLATIONS_OUTPUT/$PROJECT/Language

echo "sources dir is $SOURCES_DIR"
echo "additional dir is $ADDITIONAL_DIR"
echo "output dir is $OUTPUT_DIR"

if [ ! -f $OUTPUT_DIR ]; then
	# create output dir if it doesn't exist
	mkdir -p $OUTPUT_DIR
fi

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

echo "Carbon-copying standard files - they don't have system-specific translations."

cp -r $TRANSLATIONS_SOURCE/standard/ $TRANSLATIONS_OUTPUT

echo "All done."
