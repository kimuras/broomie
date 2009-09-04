TESTS = train_test_NB.sh classify_test_NB.sh
if ENABLE_OLL
    TESTS += train_test_oll.sh
    TESTS += classify_test_oll.sh
endif

if ENABLE_TINYSVM
    TESTS += train_test_svm.sh
    TESTS += classify_test_svm.sh
endif

EXTRA_DIST = $(TESTS) test_data
