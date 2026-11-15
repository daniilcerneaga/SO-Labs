#!/usr/bin/env bash
# Run in the directory containing the course-generated access.log.

# Q1. How many requests failed?
grep -c ' FAIL ' access.log

# Q2. Which pages were requested, and how many distinct pages are there?
cut -d' ' -f5 access.log | sort -u | awk '{print; count++} END {print count " distinct pages"}'

# Q3. How many requests did each page receive?
cut -d' ' -f5 access.log | sort | uniq -c

# Q4. Which users share the largest number of failed requests?
grep ' FAIL ' access.log | cut -d' ' -f3 | sort | uniq -c | sort -k1,1nr -k2,2 | awk 'NR==1 {maximum=$1} $1==maximum {print $2, $1}'

# Q5. How many requests did user3 make, and how many failed?
awk '$3=="user3" {total++; if($4=="FAIL") failed++} END {printf "user3: %d requests, %d failed\n", total, failed}' access.log

# Q6. Show the time and user of the last three failed requests.
grep ' FAIL ' access.log | tail -3 | cut -d' ' -f2,3

# Q7. Which login shells occur in /etc/passwd, and how many accounts use each?
cut -d: -f7 /etc/passwd | sort | uniq -c

# Q8. Compare normal and long listings of /etc; the long listing adds a total line.
ls /etc | wc -l; ls -l /etc | wc -l
