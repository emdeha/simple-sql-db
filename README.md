# Aim

For quite a while I was curious about how relational DBs are implemented. This
curiousity led me to start reading [Database Systems - The Complete
Book](https://www.amazon.com/Database-Systems-Complete-Book-2nd/dp/0131873253/ref=sr_1_4?keywords=database+system&qid=1580661596&s=books&sr=1-4)
and try to implement a simple SQL database in C.

While I'm implementing the concepts in the database I plan to provide
thoroughful learning-type documentation to my decisions in order to make my
work useful to others who'd like to dive into the implementation of DB
systems. Of course, my knowledge is currently quite limited, so I'd love it if
you submit issues and PRs with relevant suggestions.

# Constraints

The DB would be quite limited. I plan to make it support the following
queries:

* CREATE TABLE
* INSERT INTO
* SELECT ... FROM ... WHERE
* And possibly JOIN

After I do this in a reasonably sensible way, I think of continuing on other
interesting topics such as:

* Indexing
* Views
* GROUP BY
* Procedures
* and so on...
