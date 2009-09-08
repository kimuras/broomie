#!/bin/sh

echo ""
echo "*************************************************************************"
echo "    create training data test (Naive Bayes)                              "
echo "*************************************************************************"
echo ""

BRMTRAIN_CMD="../src/brmtrain"
MODEL_DIR="MODEL/"
TRAIN_DATA="test_data/train"

if [ "$1" = "-m" ]
then
   BRMTRAIN_CMD="valgrind --leak-check=full brmtrain"
fi;

# -----------------------------------------------------------------
# brmtrain -m model/ -t train economic it sport
# -----------------------------------------------------------------
echo "test > $BRMTRAIN_CMD -m $MODEL_DIR -t test_data/train economic it sport"
mkdir $MODEL_DIR
$BRMTRAIN_CMD -m $MODEL_DIR -t $TRAIN_DATA economic it sport
if [ "$?" = "0" ]
then
    echo "[[FAIL]] $BRMTRAIN_CMD -m $MODEL_DIR -t test_data/train economic it sport"
    rm -rf $MODEL_DIR
    exit -1
fi;
rm -rf $MODEL_DIR
echo ""

# -----------------------------------------------------------------
# brmtrain --model-dir model/ -t train economic it sport
# -----------------------------------------------------------------
echo "test > $BRMTRAIN_CMD --model-dir=$MODEL_DIR -t test_data/train economic it sport"
mkdir $MODEL_DIR
$BRMTRAIN_CMD --model-dir=$MODEL_DIR -t $TRAIN_DATA economic it sport
if [ "$?" = "0" ]
then
    echo "[[FAIL]] $BRMTRAIN_CMD --model-dir=$MODEL_DIR -t test_data/train economic it sport"
    rm -rf $MODEL_DIR
    exit -1
fi;
rm -rf $MODEL_DIR
echo ""

# -----------------------------------------------------------------
# brmtrain --model-dir model/ --train-data=train economic it sport
# -----------------------------------------------------------------
echo "test > $BRMTRAIN_CMD --model-dir=$MODEL_DIR --train-data=test_data/train economic it sport"
mkdir $MODEL_DIR
$BRMTRAIN_CMD --model-dir=$MODEL_DIR --train-data=$TRAIN_DATA economic it sport
if [ "$?" = "0" ]
then
    echo "[[FAIL]] $BRMTRAIN_CMD --model-dir=$MODEL_DIR --train-data=test_data/train economic it sport"
    rm -rf $MODEL_DIR
    exit -1
fi;
rm -rf $MODEL_DIR
echo ""

# -----------------------------------------------------------------
# brmtrain --model-dir=model/ --train-data=train
# -----------------------------------------------------------------
echo "test > $BRMTRAIN_CMD --model-dir=$MODEL_DIR --train-data=test_data/train"
mkdir $MODEL_DIR
$BRMTRAIN_CMD --model-dir=$MODEL_DIR --train-data=$TRAIN_DATA
if [ "$?" = "1" ]
then
    echo "[[FAIL]] $BRMTRAIN_CMD --model-dir=$MODEL_DIR --train-data=test_data/train"
    rm -rf $MODEL_DIR
    exit -1
fi;
rm -rf $MODEL_DIR
echo ""

# -----------------------------------------------------------------
# brmtrain --model-dir=model/ --train-data=train it
# -----------------------------------------------------------------
echo "test > $BRMTRAIN_CMD --model-dir=$MODEL_DIR --train-data=test_data/train it"
mkdir $MODEL_DIR
$BRMTRAIN_CMD --model-dir=$MODEL_DIR --train-data=$TRAIN_DATA it
if [ "$?" = "1" ]
then
    echo "[[FAIL]] $BRMTRAIN_CMD --model-dir=$MODEL_DIR --train-data=test_data/train it"
    rm -rf $MODEL_DIR
    exit -1
fi;
rm -rf $MODEL_DIR
echo ""

# -----------------------------------------------------------------
# brmtrain --model-dir=model/ --train-data=train it economic
# -----------------------------------------------------------------
echo "test > $BRMTRAIN_CMD --model-dir=$MODEL_DIR --train-data=test_data/train it economic"
mkdir $MODEL_DIR
$BRMTRAIN_CMD --model-dir=$MODEL_DIR --train-data=$TRAIN_DATA it sport
if [ "$?" = "1" ]
then
    echo "[[FAIL]] $BRMTRAIN_CMD --model-dir=$MODEL_DIR --train-data=test_data/train it economic"
    rm -rf $MODEL_DIR
    exit -1
fi;
rm -rf $MODEL_DIR
echo ""

# -----------------------------------------------------------------
# brmtrain -m model/ -t train -c bayes economic it sport
# -----------------------------------------------------------------
echo "test > $BRMTRAIN_CMD -m $MODEL_DIR -t test_data/train -c bayes economic it sport"
mkdir $MODEL_DIR
$BRMTRAIN_CMD -m $MODEL_DIR -t $TRAIN_DATA -e bayes economic it sport
if [ "$?" = "0" ]
then
    echo "[[FAIL]] $BRMTRAIN_CMD -m $MODEL_DIR -t test_data/train -c bayes economic it sport"
    rm -rf $MODEL_DIR
    exit -1
fi;
rm -rf $MODEL_DIR
echo ""

# -----------------------------------------------------------------
# brmtrain -m model/ -t train --classifier=bayes economic it sport
# -----------------------------------------------------------------
echo "test > $BRMTRAIN_CMD -m $MODEL_DIR -t test_data/train --classifier=bayes economic it sport"
mkdir $MODEL_DIR
$BRMTRAIN_CMD -m $MODEL_DIR -t $TRAIN_DATA --classifier=bayes economic it sport
if [ "$?" = "0" ]
then
    echo "[[FAIL]] $BRMTRAIN_CMD -m $MODEL_DIR -t test_data/train --classifier=bayes economic it sport"
    rm -rf $MODEL_DIR
    exit -1
fi;
rm -rf $MODEL_DIR
echo ""

# -----------------------------------------------------------------
# brmtrain -m model/ -t train -c perceptron economic it sport
# -----------------------------------------------------------------
echo "test > $BRMTRAIN_CMD -m $MODEL_DIR -t test_data/train -c perceptron  economic it sport"
mkdir $MODEL_DIR
$BRMTRAIN_CMD -m $MODEL_DIR -t $TRAIN_DATA -c perceptron economic it sport
if [ "$?" = "1" ]
then
    echo "[[FAIL]] $BRMTRAIN_CMD -m $MODEL_DIR -t test_data/train -c perceptron economic it sport"
    rm -rf $MODEL_DIR
    exit -1
fi;
rm -rf $MODEL_DIR
echo ""

exit 0
