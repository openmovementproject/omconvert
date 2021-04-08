import csv
import re
import os
import datetime
import array


# For files with relative numeric timestamps that store the base time in the filename
def csv_time_from_filename(csv_filename):
    time_zero = 0
    parts = re.match('(\d\d\d\d)-?(\d\d)-?(\d\d)[-T ]?(\d\d):?(\d\d):?(\d\d)(\.?\d\d\d)?', os.path.basename(csv_filename))
    if parts:
        msec = parts.group(7)
        if msec == None:
            msec = 0
        else:
            msec = int(msec)
        time_zero = datetime.datetime(int(parts.group(1)), int(parts.group(2)), int(parts.group(3)), int(parts.group(4)), int(parts.group(5)), int(parts.group(6)), msec * 1000, tzinfo=datetime.timezone.utc).timestamp()
    return time_zero


# For files where the scaling metadata is stored int the last part of the filename
def csv_scale_from_filename(csv_filename):
    global_scale = 1
    basename = os.path.splitext(os.path.basename(csv_filename))[0]
    if basename.endswith('_ACC'):
        global_scale = 0.0002   # really 1/5000? Or possibly 1/4096?
    if basename.endswith('_GYR'):
        global_scale = 0.0305   # probably 1/32768
    if basename.endswith('_EEG') or basename.endswith('_EOG'):
        global_scale = 0.4808
    if basename.endswith('_ECG'):
        global_scale = 1.4424
    if basename.endswith('_EMG'):
        global_scale = 0.4808
    return global_scale


# Convert a timestamp-with-no-timezone into a datetime (using UTC even though unknown zone, alternative is naive datetime which is assumed to be in the current local computer's time)
def csv_datetime(timestamp):
    return datetime.datetime.fromtimestamp(timestamp, tz=datetime.timezone.utc)


# Convert a timestamp-with-no-timezone into a ISO-ish string representation (using UTC even though unknown zone, alternative is naive datetime which is assumed to be in the current local computer's time)
def csv_datetime_string(time):
    if not isinstance(time, datetime.datetime):
        time = csv_datetime(time)
    return time.isoformat(sep=' ',timespec='milliseconds')[0:23]


def csv_load(csv_filename, options = {}):
    """
    Load a .CSV file.
    The first row can contain column headers.
    If the first data column contains an ISO-ish timestamp with no offset, it is parsed as a time in seconds since the 1970 epoch date
    -- importantly, no timezone information is known, so treat as a UTC time to correctly recover date/time of day.
    All other values must be double-parsable.
    Each row is returned as a double array of the same length as the first data row in the file.
    The result is a tuple of the header cells, and a list of rows.
    """

    # For relative numeric timestamps, the epoch time for zero
    time_zero = options.get('time_zero') or 0

    # An overall timstamp offset (e.g. to convert to a time zone)
    time_offset = options.get('time_offset') or 0

    # Apply scaling for columns other than the first column
    global_scale = options.get('global_scale') or 1

    with open(csv_filename, newline='') as csv_file:
        initial_chunk = csv_file.read(1024)
        sniffer = csv.Sniffer()
        dialect = sniffer.sniff(initial_chunk)
        has_header = sniffer.has_header(initial_chunk)

        csv_file.seek(0)
        csv_reader = csv.reader(csv_file, dialect) # quoting=csv.QUOTE_NONNUMERIC

        header = None
        if has_header:
            header = csv_reader.__next__()

        first_row = csv_reader.__next__()
        num_columns = len(first_row)
        empty_row = [0] * num_columns

        # Check whether the first column in a formatted date/time
        date_time_re = re.compile('^\d\d\d\d-\d\d-\d\d[T ]\d\d:\d\d:\d\d\.?(?:\d\d\d)?$')
        initial_date_time = date_time_re.match(first_row[0])

        # Reset to first row
        csv_file.seek(0)
        if has_header:
            csv_reader.__next__()
        
        values = []
        for row in csv_reader:
            row_values = array.array('d', empty_row)
            
            # First column is time
            if initial_date_time:
                # Seconds since epoch but no timezone
                timestamp = datetime.datetime.fromisoformat(row[0] + '+00:00').timestamp()
                row_values[0] = timestamp + time_offset
            else:
                row_values[0] = float(row[0]) + time_zero + time_offset

            # Remaining columns are data, potentially scaled
            for i in range(1, len(row)):
                if i < num_columns:
                    row_values[i] = float(row[i]) * global_scale
            
            values.append(row_values)
        
        return (header, values)


def main():
    test_file = '_local/2021-04-01-123456123_XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX_ACC.csv'
    #test_file = '_local/sample.csv'
    (header, values) = csv_load(test_file, {"time_zero": csv_time_from_filename(test_file), "global_scale": csv_scale_from_filename(test_file)})
    print(header)
    print(len(values))
    for i in range(0, 10):
        if i >= len(values):
            break
        time_string = csv_datetime_string(values[i][0])
        print('#' + str(i) + ' @' + time_string + ' = ' + str(values[i]))

if __name__ == "__main__":
    main()
