#!/bin/sh

echo ""
echo "*************************************************************************"
echo "    classify test (BAYES)                                                "
echo "*************************************************************************"
echo ""

BRMTRAIN_CMD="../src/brmtrain"
BRMCLASSIFY_CMD="../src/brmclassify"
MODEL_DIR="MODEL/"
TRAIN_DATA="test_data/train"
LABELED_TEST_DATA="test_data/test_labeled"
NOLABLE_TEST_DATA="test_data/test_nolabel"
SHORT_LABELED_TEST_DATA="test_data/test_labeled.test"

if [ "$1" = "-m" ]
then
    BRMCLASSIFY_CMD="valgrind --leak-check=full brmclassify"
fi;

# -----------------------------------------------------------------
# create model
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
echo ""

# -----------------------------------------------------------------
# brmclassify accuracy -m model/ test_labeled
# -----------------------------------------------------------------
echo "test > $BRMCLASSIFY_CMD accuracy -m $MODEL_DIR -t $LABELED_TEST_DATA"
$BRMCLASSIFY_CMD accuracy -m $MODEL_DIR -t $LABELED_TEST_DATA
if [ "$?" = "0" ]
then
    echo "[[FAIL]] $BRMCLASSIFY_CMD -m $MODEL_DIR -t $LABELED_TEST_DATA"
    rm -rf $MODEL_DIR
    exit -1
fi;
echo ""

# -----------------------------------------------------------------
# brmclassify classify -m model/ -t test_nolabel
# -----------------------------------------------------------------
echo "test > $BRMCLASSIFY_CMD classify -m $MODEL_DIR -t $NOLABLE_TEST_DAT"
$BRMCLASSIFY_CMD classify -m $MODEL_DIR -t $NOLABLE_TEST_DATA
if [ "$?" = "0" ]
then
    echo "$BRMCLASSIFY_CMD classify -m $MODEL_DIR -t $NOLABEL_TEST_DATA"
    rm -rf $MODEL_DIR
    exit -1
fi;
echo ""

# -----------------------------------------------------------------
# brmclassify accuracy -pv -m model/ -t test_labled.test
# -----------------------------------------------------------------
echo "test > $BRMCLASSIFY_CMD accuracy -pv -m $MODEL_DIR -t $LABELED_TEST_DATA"
$BRMCLASSIFY_CMD accuracy -pv -m $MODEL_DIR -t $SHORT_LABELED_TEST_DATA
if [ "$?" = "0" ]
then
    echo "[[FAIL]] $BRMCLASSIFY_CMD -m $MODEL_DIR -t $LABELED_TEST_DATA"
    rm -rf $MODEL_DIR
    exit -1
fi;
echo ""

# -----------------------------------------------------------------
# brmclassify accuracy -m model/ -t test_labled.test
# -----------------------------------------------------------------
echo "test > $BRMCLASSIFY_CMD accuracy -m $MODEL_DIR -t $SHORT_LABELED_TEST_DATA"
$BRMCLASSIFY_CMD accuracy -m $MODEL_DIR -t $SHORT_LABELED_TEST_DATA
if [ "$?" = "0" ]
then
    echo "[[FAIL]] $BRMCLASSIFY_CMD -m $MODEL_DIR -t $LABELED_TEST_DATA"
    rm -rf $MODEL_DIR
    exit -1
fi;
echo ""

# -----------------------------------------------------------------
# brmclassify accuracy --model-dir=model --test-data=test_labled.test
# -----------------------------------------------------------------
echo "test > $BRMCLASSIFY_CMD accuracy --model-dir=$MODEL_DIR --test-data=$SHORT_LABELED_TEST_DATA"
$BRMCLASSIFY_CMD accuracy --model-dir=$MODEL_DIR --test-data=$SHORT_LABELED_TEST_DATA
if [ "$?" = "0" ]
then
    echo "[[FAIL]] $BRMCLASSIFY_CMD accuracy --model-dir=$MODEL_DIR --test-data=$SHORT_LABELED_TEST_DATA"
    rm -rf $MODEL_DIR
    exit -1
fi;

# -----------------------------------------------------------------
# brmclassify accuracy -pv --model-dir=model --test-data=test_labled.test
# -----------------------------------------------------------------
echo "test > $BRMCLASSIFY_CMD accuracy -pv --model-dir=$MODEL_DIR --test-data=$SHORT_LABELED_TEST_DATA"
$BRMCLASSIFY_CMD accuracy -pv --model-dir=$MODEL_DIR --test-data=$SHORT_LABELED_TEST_DATA
if [ "$?" = "0" ]
then
    echo "[[FAIL]] $BRMCLASSIFY_CMD accuracy -pv --model-dir=$MODEL_DIR --test-data=$SHORT_LABELED_TEST_DATA"
    rm -rf $MODEL_DIR
    exit -1
fi;

rm -rf $MODEL_DIR
