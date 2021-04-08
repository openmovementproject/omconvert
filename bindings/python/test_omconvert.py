import os
import omconvert

def main():
    om = omconvert.OmConvert()

    source_file = '_local/sample.cwa'

    # Base path and file name without extension
    base_name = os.path.splitext(source_file)[0]

    options = {}
    #options['executable'] = '/path/to/omconvert'
    options['csv_file'] = base_name + '.csv'
    options['svm_file'] = base_name + '.svm.csv'
    options['wtv_file'] = base_name + '.wtv.csv'
    options['paee_file'] = base_name + '.paee.csv'

    result = om.execute(source_file, options)

    # for key, value in result.items():
    #     print('RESULT: ' + key + ' == ' + str(value))


if __name__ == "__main__":
    main()
