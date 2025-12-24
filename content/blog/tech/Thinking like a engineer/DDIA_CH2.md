# Data Models

These are the base layers of any application, based on that the software is designed. Relation databases have been the forerunner in operating of how data models should structure and what operations can be performed on them, but in recent years, there are new players like document based databases or noSql and Graph based databases.

Data models is a template , more like a schema, but with how data is collected for the applicaton, how it should be structured, and the rules it should follow. 

Without data models the organization could face a lot of issues in production , and it will not be easy to solve critical issues. 

Data models define entities and its relationships with each other, these entites could be rows, and relationships could be columns in a relational database.

# Normalization

The idea to remove redundancies from the database is known as normalization.

In relational DB we could see 4 kinds of normalization :

## 1NF(first normal form)

where each column contains atomic values, no repeating groups or arrays.

## 2NF(Second normal form)

It must be in 1NF, and there should be no partial dependencies (all non-key attributes depend on the entire primary key)

## 3NF(Third normal form)

It must be in 2NF, and there should be no transitive dependencies (non-key attributes shouldn’t depend on other non-key attributes)

## (Boyle-Codd normal form)

Stricter version of 3NF, where every determinant must be a candidate key.

## Example:

## Unnormalized Form (0NF)

**Student_Course_Data:**

| student_id | student_name | courses_enrolled | instructors |
| --- | --- | --- | --- |
| 1 | Lokesh | IST821, IST597 | Prof.Dan, Prof.Smith |
| 2 | Alice | IST821 | Prof.Dan |

**Problems:** Repeating groups (multiple courses in one field), non-atomic values

---

## 1NF (Atomic Values)

**Student_Course:**

| student_id | student_name | course_id | instructor |
| --- | --- | --- | --- |
| 1 | Lokesh | IST821 | Prof.Dan |
| 1 | Lokesh | IST597 | Prof.Smith |
| 2 | Alice | IST821 | Prof.Dan |

✓ **Fixed:** All values are atomic

✗ **Problem:** student_name repeats (redundancy), instructor depends only on course_id (partial dependency)

---

## 2NF (No Partial Dependencies)

**Students:**

| student_id | student_name |
| --- | --- |
| 1 | Lokesh |
| 2 | Alice |

**Enrollments:**

| student_id | course_id |
| --- | --- |
| 1 | IST821 |
| 1 | IST597 |
| 2 | IST821 |

**Courses:**

| course_id | instructor |
| --- | --- |
| IST821 | Prof.Dan |
| IST597 | Prof.Smith |

✓ **Fixed:** No partial dependencies

✗ **Problem:** If Prof.Dan teaches multiple courses, his name repeats (transitive dependency: course_id → instructor)

---

## 3NF (No Transitive Dependencies)

**Students:**

| student_id | student_name |
| --- | --- |
| 1 | Lokesh |
| 2 | Alice |

**Enrollments:**

| student_id | course_id |
| --- | --- |
| 1 | IST821 |
| 1 | IST597 |
| 2 | IST821 |

**Courses:**

| course_id | instructor_id |
| --- | --- |
| IST821 | 101 |
| IST597 | 102 |

**Instructors:**

| instructor_id | instructor_name |
| --- | --- |
| 101 | Prof.Dan |
| 102 | Prof.Smith |

✓ **Fully normalized:** No redundancy, each fact stored once, easy updates!

# Relational Databases vs Document Database(Nosql)

## Many to Many relationships

In Document databases which relies on JSON and XML to represent data model and form relationships, while relational database relies on tables, like we seen above. It’s easier to represent Many to Many relationship in RDBs than in Nosql. 

Relationship : Posts↔ Tags( one post has many tags, one tag appears in many posts). 

SQL(Relational): where we can have a posts table, tags table, and a junction table that has both posts and tags id, so with a single join command you can either get posts with particular tags, or tags related to particular posts.

NoSQL : we might embed tags in posts, so, each posts inherently contain tags like “AI”, “security”. so when we try to find all posts with tag “AI”, it becomes cumbersome. We might either use **Denormalize**( duplicate data)→ consistency Problems and Use **references**(like SQL foreign keys) → performance problems. where if you want to update a post that contains tag “AI”, but now you want to change the tag to “artificial Intelligence”, what happens is, you have to write application code to do additional work to keep denormalized data consistent— risks you have is ( race conditions, partial failures ). If you try to use **references** , then there would be multiple reads and network round trips, more memory, complex error handling and the code gets bloated.

These problems could be seen across in Many to One too. But in One to many both SQL and Nosql do better job.

## Summary

| Relationship | SQL | NoSQL |
| --- | --- | --- |
| **One-to-Many** | Easy (foreign key) | Easy (embedding) |
| **Many-to-One** | Easy (JOIN) | Hard (scan or duplicate) |
| **Many-to-Many** | Easy (junction table) | Hard (no native joins, duplicate or multiple queries) |

## Schema-on-write and Schema-on-read

**Schema-on-read** is associated with Nosql, here the schema is implicitly defined and only interpreted when data is read. It’s similar to runtime type checking , here type means schema.

**Schema-on-write** is associated with relational DB, where the schemas are defined explicitly and ensures that the data written conforms to it.

## Schema Migrations

Changes in Schemas like adding new column, might cause schema migrations, which is easier in Nosql as you have to write few lines of code in the application and move on from there.

Unfortunately relational databases can too, except Mysql, which when the table alters, copies the whole table before updating the column or schema, though now, there are workarounds like githubs ghost migration.

## Locality

Locality is refered to how related data that’s frequently accessed together is stored physically close together in disk. Good locality means that data you require is in one place, poor locality means data is scattered across disk leading to slow reads or updates.

In **relational DBs** , clustered indexes, index organized tables and partioning are used for better locality. It is useful if you want to access data together.

In **Document DBs** , Sharding, compression, and partial updates where fields are updated without writing the entire documents. It’s useful if you want to access data in many different combinations.

# Network model

In network model, there is a root from which the data is transferred to a leaf node, it become cumbersome over time. To fetch a data in its intermediate node or in leaf node, you have to have a access path, which fetches the data through traversing from root to the specified node. It creates lag when updating the database or when issuing multiple queries.

# Query Language for data

There are two types of query language:

1. Declarative QL ; SQL, its also used in CSS and XSL too, for parent child relationships. it is more attractive (concise and could be developed for performance) and easy to parallelize as it doesn’t depend on order.
2. Imperative QL : a code or program, more like nosql. It could be cumbersome, because it follows order, where it needs to change or move around the data to reclaim free space. An imperative language tells the computer to perform certain operations in certain order.

# Mapreduce

It’s used primarily for querying large databases with more data. It parallelizes across many machines by breaking work into two phases : Map(transform) and reduce(aggregate), where map transforms to a utility which could be then reduced to get the appropriate result.

# Graph like Data Models

Mostly used for many to many relationships.

A graph contains 2 kinds of objects:

1. Vertex
2. Edges

Examples of where it could be used are in social graphs, the web graph and in road or rail networks.

## Graph Models

### Property graph model

It’s used in neo4j, titan and infinegraph. Here the vertex and edge consist of a unique ID and a collection of properties(key-value), it differs when vertex has a set of outgoing and incoming edges, while edges have tail vertex(start point) and head vertex(end point).

It’s more like two relational DBs, but it has no schemas similar to Nosql, which is suitable to trace out connectivity, and it could have different relationships with different vertices.

### Triple Store model

It’s used in Datomic, Allegrograph and athens. Here the informations are stored in very simple three part statements : (**subject**, **predicate**, **object** ) for eg: (jim, likes, bananas). 

The subject of a triple is equivalent to a vertex in graph and object could be either a primitive data type(lucy, age, 25) or another vertex in graph(lucy, married, alie). These were mainly used for semantic web, with resource description framework.

### Query Languages

**Cypher** → it’s a declarative query language, created for Neo4j graph DB, it works by starting with two vertices and can goes backward.. it takes constant time.

**SPARQL → used for RDF.**

**Datalog** → oldest graph query language used in hadoop, and dataomic.

# Graph queries in SQL

You have to traverse graph before you reach your answers and it may take many no of joins, which consumes a lot of memory and computing power.

# Can I use graph data models for Many to one and one to many:

It works fine, but it depends on my use case, as graph models are good at complex traversals and connected data, relational databases provide simple queries and aggregations. 

Graph models would be overkill for simple one to many , while sql provides deeply connected variable-depth queries.

If you are concerned only about the relationships, then go for graph models, but if your relationships are just organizational (foreign keys for grouping), use SQL.