#!/bin/sh

BRMTRAIN_CMD="../src/brmtrain"
MODEL_DIR="MODEL/"
TRAIN_DATA="test_data/train"

if [ "$1" == "-m" ]
then
   BRMTRAIN_CMD="valgrind --leak-check=full brmtrain"
fi;

mkdir $MODEL_DIR
$BRMTRAIN_CMD -m $MODEL_DIR -t $TRAIN_DATA economic it sport
if [ "$?" == "0" ]
then
    echo "[[FAIL]] $BRMTRAIN_CMD -m $MODEL_DIR -t test_data/train economic it sport"
    rm -rf $MODEL_DIR
    exit -1
fi;
rm -rf $MODEL_DIR

# -----------------------------------------------------------------

mkdir $MODEL_DIR
$BRMTRAIN_CMD --model-dir=$MODEL_DIR -t $TRAIN_DATA economic it sport
if [ "$?" == "0" ]
then
    echo "[[FAIL]] $BRMTRAIN_CMD --model-dir=$MODEL_DIR -t test_data/train economic it sport"
    rm -rf $MODEL_DIR
    exit -1
fi;
rm -rf $MODEL_DIR

# -----------------------------------------------------------------

mkdir $MODEL_DIR
$BRMTRAIN_CMD --model-dir=$MODEL_DIR --train-data=$TRAIN_DATA economic it sport
if [ "$?" == "0" ]
then
    echo "[[FAIL]] $BRMTRAIN_CMD --model-dir=$MODEL_DIR --train-data=test_data/train economic it sport"
    rm -rf $MODEL_DIR
    exit -1
fi;
rm -rf $MODEL_DIR

# -----------------------------------------------------------------

mkdir $MODEL_DIR
$BRMTRAIN_CMD --model-dir=$MODEL_DIR --train-data=$TRAIN_DATA
if [ "$?" == "1" ]
then
    echo "[[FAIL]] $BRMTRAIN_CMD --model-dir=$MODEL_DIR --train-data=test_data/train"
    rm -rf $MODEL_DIR
    exit -1
fi;
rm -rf $MODEL_DIR

# -----------------------------------------------------------------

mkdir $MODEL_DIR
$BRMTRAIN_CMD --model-dir=$MODEL_DIR --train-data=$TRAIN_DATA it
if [ "$?" == "1" ]
then
    echo "[[FAIL]] $BRMTRAIN_CMD --model-dir=$MODEL_DIR --train-data=test_data/train it"
    rm -rf $MODEL_DIR
    exit -1
fi;
rm -rf $MODEL_DIR

# -----------------------------------------------------------------

mkdir $MODEL_DIR
$BRMTRAIN_CMD --model-dir=$MODEL_DIR --train-data=$TRAIN_DATA it sport
if [ "$?" == "1" ]
then
    echo "[[FAIL]] $BRMTRAIN_CMD --model-dir=$MODEL_DIR --train-data=test_data/train it economic"
    rm -rf $MODEL_DIR
    exit -1
fi;
rm -rf $MODEL_DIR

# -----------------------------------------------------------------


exit 0
