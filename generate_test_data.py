from datetime import *
from random import *
import string
import numpy as np

# Generate Test Data with results
# Generate random URLs
def random_url():
    """
    This function will return a random url.
    """
    letters = string.ascii_lowercase
    tld_letters = ["c","o","m","i","u","d","e","p","k",'q','a']
    domain_len = randint(2,10)
    domain = "".join(choice(letters) for i in range(domain_len))
    bld_len = randint(0,3)
    bld = "".join(choice(letters) for i in range(bld_len))
    tld_len = randint(2,3)
    tld = "".join(choice(tld_letters) for i in range(tld_len))
    return "www."+domain + "." + (tld if bld == "" else bld+"."+tld)

# Generate random Dates
def random_date(start, end):
    """
    This function will return a random datetime between two datetime 
    objects.
    """
    delta = end - start
    int_delta = (delta.days * 24 * 60 * 60) + delta.seconds
    random_second = randrange(int_delta)
    return (start + timedelta(seconds=random_second)).strftime("%d/%m/%Y")

#Generate a date obj form a string
def date_string(string):
    return datetime.strptime(string, "%d/%m/%Y")

# Generate test data
def generate_test_data(start, end, numb_entries):
    """
    This function will return a set of dates and domains of size numb_entries.
    """
    begin = datetime.strptime(str(start), "%d/%m/%Y")
    end = datetime.strptime(str(end), "%d/%m/%Y")

    # Define number of urls
    max_data = numb_entries
    test_data = np.empty((max_data,2), dtype=object)

    for x in range(0,max_data):
        test_data[x] = (random_date(begin,end), random_url())
    
    return test_data

# Output To Files
def output_test_data(filename, test_data, narrow_start=None, narrow_end=None):
    # Write to file
    with open(filename+".txt", "w") as f:
        for data in test_data:
            f.write(data[0] + " " + data[1] + "\n")

    # Now to output percentages
    stats = {}

    # Count times a tld appears
    for data in test_data:
        if narrow_start != None and narrow_end != None:
            if date_string(data[0]) >= date_string(narrow_start) and date_string(data[0]) <= date_string(narrow_end):
                stats[str(data[1]).split(".")[-1]] = stats.get(str(data[1]).split(".")[-1], 0) + 1
        else:
            stats[str(data[1]).split(".")[-1]] = stats.get(str(data[1]).split(".")[-1], 0) + 1

    # Output results into a readable format
    sortedstats = { k:v for k,v in sorted(stats.items(), key=lambda item: item[1]) }
    total = sum(sortedstats.values())
    with open(filename+".out", "w") as f:
        for (key,value) in sortedstats.items():
            f.write(str(round((value/total)*100, 2)) + " " + key + " " + str(value) +"\n")

    print(total)

test_data = generate_test_data("12/01/1999","20/02/2020", 1000000)
output_test_data("test", test_data, "12/07/2017", "12/07/2018")
#output_test_data("test", test_data)
