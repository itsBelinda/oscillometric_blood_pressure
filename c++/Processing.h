#ifndef OBP_PROCESSING_H
#define OBP_PROCESSING_H

#include "CppThread.h"

#define DEFAULT_MINUTES 2
#define DEFAULT_DATA_SIZE (1024 * 60 * DEFAULT_MINUTES)


class Processing : public CppThread {
public:
    Processing();
    ~Processing() override;


    bool bSaveToFile();

private:
    void run() override;
    vector<double> pData(DEFAULT_DATA_SIZE);
    vector<double> oData(DEFAULT_DATA_SIZE);

};


#endif //OBP_PROCESSING_H
