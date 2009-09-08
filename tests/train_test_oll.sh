#!/bin/sh

echo ""
echo "*************************************************************************"
echo "    create training data test (oll)                                      "
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
# brmtrain -m model/ -t train -c oll economic it sport
# -----------------------------------------------------------------
echo "test > $BRMTRAIN_CMD -m $MODEL_DIR -t test_data/train -c oll economic it sport"
mkdir $MODEL_DIR
$BRMTRAIN_CMD -m $MODEL_DIR -t $TRAIN_DATA -c oll economic it sport
if [ "$?" = "0" ]
then
    echo "[[FAIL]] $BRMTRAIN_CMD -m $MODEL_DIR -t test_data/train -c oll economic it sport"
    rm -rf $MODEL_DIR
    exit -1
fi;
rm -rf $MODEL_DIR
echo ""

exit 0
