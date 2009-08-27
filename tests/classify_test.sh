#!/bin/sh

BRMTRAIN_CMD="../src/brmtrain"
BRMCLASSIFY_CMD="../src/brmclassify"
MODEL_DIR="MODEL/"
TRAIN_DATA="test_data/train"
LABELED_TEST_DATA="test_data/test_labeled"
NOLABLE_TEST_DATA="test_data/test_nolabel"
TMP="test_data/test_labeled.test"

if [ "$1" == "-m" ]
then
    BRMCLASSIFY_CMD="valgrind --leak-check=full brmclassify"
fi;

mkdir $MODEL_DIR
$BRMTRAIN_CMD -m $MODEL_DIR -t $TRAIN_DATA economic it sport
if [ "$?" == "0" ]
then
    echo "[[FAIL]] $BRMTRAIN_CMD -m $MODEL_DIR -t test_data/train economic it sport"
    rm -rf $MODEL_DIR
    exit -1
fi;

# -----------------------------------------------------------------

$BRMCLASSIFY_CMD accuracy -m $MODEL_DIR -t $LABELED_TEST_DATA
if [ "$?" == "0" ]
then
    echo "[[FAIL]] $BRMCLASSIFY_CMD -m $MODEL_DIR -t $LABELED_TEST_DATA"
    rm -rf $MODEL_DIR
    exit -1
fi;

$BRMCLASSIFY_CMD classify -m $MODEL_DIR -t $NOLABLE_TEST_DATA
if [ "$?" == "0" ]
then
    echo "$BRMCLASSIFY_CMD classify -m $MODEL_DIR -t $NOLABEL_TEST_DATA"
    rm -rf $MODEL_DIR
    exit -1
fi;

# -----------------------------------------------------------------

$BRMCLASSIFY_CMD accuracy -pv -m $MODEL_DIR -t $TMP
if [ "$?" == "0" ]
then
    echo "[[FAIL]] $BRMCLASSIFY_CMD -m $MODEL_DIR -t $LABELED_TEST_DATA"
    rm -rf $MODEL_DIR
    exit -1
fi;

# -----------------------------------------------------------------

$BRMCLASSIFY_CMD accuracy -m $MODEL_DIR -t $TMP
if [ "$?" == "0" ]
then
    echo "[[FAIL]] $BRMCLASSIFY_CMD -m $MODEL_DIR -t $LABELED_TEST_DATA"
    rm -rf $MODEL_DIR
    exit -1
fi;

# -----------------------------------------------------------------

rm -rf $MODEL_DIR
