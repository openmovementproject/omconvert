from calc_svm import CalcSvm
from timeseries_csv import *

def main():
    #test_file = '_local/2021-04-01-123456123_XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX_ACC.csv'
    test_file = '_local/sample.csv'
    tscsv = TimeseriesCsv(test_file, {"time_zero": csv_time_from_filename(test_file), "global_scale": csv_scale_from_filename(test_file)})
    #print(tscsv.header)
    svm_calc = CalcSvm(tscsv, {})

    print("Time,Mean SVM (g)")
    for time, svm in svm_calc:
        #time_dt = csv_datetime(time)
        time_string = csv_datetime_string(time, False)
        print(time_string + ',' + str(svm))


if __name__ == "__main__":
    main()
