DROP DATABASE IF EXISTS job_db;
CREATE DATABASE job_db;

\c job_db;
DROP SCHEMA IF EXISTS jobs CASCADE;
CREATE SCHEMA jobs;

CREATE TABLE jobs.Applicant (
                                FirstName VARCHAR(100),
                                LastName VARCHAR(100),
                                EMPLID VARCHAR(100),
                                PRIMARY KEY (EMPLID)
);

CREATE TABLE jobs.PositionPosting (
                                      PostingID SERIAL,
                                      PostingName VARCHAR(255),
                                      Pay DECIMAL(18,2),
                                      PRIMARY KEY (PostingID)
);

CREATE TABLE jobs.Application (
                                  ApplicationID SERIAL,
                                  PostingID INT,
                                  EMPLID VARCHAR(100),
                                  PAFLink VARCHAR(255),
                                  SignatureCount int,
                                  PRIMARY KEY (ApplicationID),
                                  FOREIGN KEY (EMPLID) REFERENCES jobs.Applicant(EMPLID),
                                  FOREIGN KEY (PostingID) REFERENCES jobs.PositionPosting(PostingID)
);

INSERT INTO jobs.PositionPosting (PostingName, Pay) VALUES
                                                        ('Math Tutor', 18.50),
                                                        ('Library Assistant', 15.00),
                                                        ('Computer Lab Technician', 20.00),
                                                        ('Administrative Assistant', 17.25),
                                                        ('Student Success Coach', 19.75);

INSERT INTO jobs.Applicant (FirstName, LastName, EMPLID) VALUES
                                                             ('Yang', 'Smith', 'EMPL001'),
                                                             ('Mei', 'Johnson', 'EMPL002'),
                                                             ('Nuria', 'Garcia', 'EMPL003'),
                                                             ('Lana', 'Davis', 'EMPL004');
