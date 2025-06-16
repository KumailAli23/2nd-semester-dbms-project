-- CRITICAL: Switch to the correct database BEFORE creating tables
USE DBMS_PROJECT;
GO

USE DBMS_PROJECT;
GO

-- Step 1: Drop existing tables in the correct order (reverse of creation)
-- This makes the script re-runnable for easy testing and reset.
DROP TABLE IF EXISTS Animal_Training_Trainer;
DROP TABLE IF EXISTS Training_Trainer;
DROP TABLE IF EXISTS Animal;
DROP TABLE IF EXISTS Worker;
DROP TABLE IF EXISTS Trainer;
DROP TABLE IF EXISTS Employee;
DROP TABLE IF EXISTS Training;
DROP TABLE IF EXISTS Allowed_Animal;
DROP TABLE IF EXISTS Customer;
GO

-- Step 2: Create the tables with robust features

CREATE TABLE Customer (
    CNIC VARCHAR(20) PRIMARY KEY,
    Name VARCHAR(255),
    Address VARCHAR(255),
    PhoneNumber VARCHAR(20)
);

CREATE TABLE Allowed_Animal (
    Animal_Type VARCHAR(255) PRIMARY KEY
);

CREATE TABLE Employee (
    Emp_id INT PRIMARY KEY IDENTITY(101,1),
    CNIC VARCHAR(20) UNIQUE, -- Enforce that CNIC must be unique per employee
    Name VARCHAR(255),
    WorkerOrTrainer VARCHAR(255),
    Address VARCHAR(255)
);

CREATE TABLE Trainer (
    Trainer_id INT PRIMARY KEY,
    Age INT,
    Phone VARCHAR(13),
    -- If an Employee record is deleted, delete the corresponding Trainer record.
    -- If an Employee's Emp_id is updated, update it here too.
    FOREIGN KEY (Trainer_id) REFERENCES Employee(Emp_id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);

CREATE TABLE Worker (
    Worker_id INT PRIMARY KEY,
    WorkerType VARCHAR(255),
    Age INT,
    -- If an Employee record is deleted, delete the corresponding Worker record.
    -- If an Employee's Emp_id is updated, update it here too.
    FOREIGN KEY (Worker_id) REFERENCES Employee(Emp_id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);

CREATE TABLE Training (
    Training_id INT PRIMARY KEY IDENTITY(1,1),
    Name VARCHAR(255)
);

CREATE TABLE Training_Trainer (
    TT_id INT PRIMARY KEY IDENTITY(1001,1),
    Training_id INT,
    Trainer_id INT,
    -- If a Training or Trainer is deleted/updated, reflect the change here.
    FOREIGN KEY (Training_id) REFERENCES Training(Training_id)
        ON DELETE CASCADE
        ON UPDATE CASCADE,
    FOREIGN KEY (Trainer_id) REFERENCES Trainer(Trainer_id)
        -- NOTE: We don't cascade from Trainer because Employee already handles it.
        -- Deleting the Employee will cascade to Trainer, which will cascade here.
);

CREATE TABLE Animal (
    Animal_id INT PRIMARY KEY IDENTITY(1,1),
    AnimalType VARCHAR(255),
    CNIC VARCHAR(20),
    Name VARCHAR(255),
    Age INT,
    StartDate DATE,
    ReturnDate DATE,
    -- If an Allowed_Animal type is updated, update it here. (Deletion is restricted by default)
    FOREIGN KEY (AnimalType) REFERENCES Allowed_Animal(Animal_Type)
        ON UPDATE CASCADE,
    -- If a Customer is deleted or their CNIC is updated, reflect it here.
    FOREIGN KEY (CNIC) REFERENCES Customer(CNIC)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);

CREATE TABLE Animal_Training_Trainer (
    Animal_id INT,
    TT_id INT,
    PRIMARY KEY (Animal_id, TT_id),
    -- If an Animal is deleted/updated, remove/update the training link.
    FOREIGN KEY (Animal_id) REFERENCES Animal(Animal_id)
        ON DELETE CASCADE
        ON UPDATE CASCADE,
    -- If a Training_Trainer link is deleted/updated, remove/update the link here.
    FOREIGN KEY (TT_id) REFERENCES Training_Trainer(TT_id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);
GO

GO


-- =================================================================================
-- CORRECTED Sample Data Insertion for DBMS_PROJECT with IDENTITY columns
--
-- Instructions:
-- 1. Make sure you have already run the CREATE TABLE script with IDENTITY columns.
-- 2. Run this entire script. It uses SET IDENTITY_INSERT to allow explicit IDs
--    for the sample data, which is necessary to maintain the relationships
--    between the hardcoded sample records.
-- =================================================================================

USE DBMS_PROJECT;
GO

-- Step 1: Populate independent tables first
-----------------------------------------------------------------------------------

PRINT 'Inserting into Allowed_Animal...';
INSERT INTO Allowed_Animal (Animal_Type) VALUES
('Dog'),
('Cat'),
('Parrot'),
('Rabbit');

PRINT 'Inserting into Customer...';
INSERT INTO Customer (CNIC, Name, Address, PhoneNumber) VALUES
('11111-1111111-1', 'John Doe', '123 Oak Street, Anytown', '555-0101'),
('22222-2222222-2', 'Jane Smith', '456 Pine Avenue, Somecity', '555-0102'),
('33333-3333333-3', 'Peter Jones', '789 Maple Lane, Yourtown', '555-0103');

-- *** CORRECTED: Must enable IDENTITY_INSERT for the Training table ***
PRINT 'Inserting into Training...';
SET IDENTITY_INSERT Training ON;
INSERT INTO Training (Training_id, Name) VALUES
(1, 'Basic Obedience'),
(2, 'Agility Course'),
(3, 'Guard Dog Training'),
(4, 'Advanced Tricks');
SET IDENTITY_INSERT Training OFF;

-- *** CORRECTED: Must enable IDENTITY_INSERT for the Employee table ***
PRINT 'Inserting into Employee...';
SET IDENTITY_INSERT Employee ON;
INSERT INTO Employee (Emp_id, CNIC, Name, WorkerOrTrainer, Address) VALUES
(101, '44444-4444444-4', 'Alice Williams', 'Trainer', '210 Birch Road'),
(102, '55555-5555555-5', 'Bob Johnson', 'Trainer', '211 Birch Road'),
(201, '66666-6666666-6', 'Charlie Brown', 'Worker', '345 Cedar Blvd'),
(202, '77777-7777777-7', 'Diana Prince', 'Worker', '346 Cedar Blvd');
SET IDENTITY_INSERT Employee OFF;


-- Step 2: Populate tables that depend on the first set
-----------------------------------------------------------------------------------
-- NOTE: Trainer and Worker tables do NOT have IDENTITY columns, so they don't need the special command.
-- Their primary keys are just foreign keys.

PRINT 'Inserting into Trainer (specialization of Employee)...';
INSERT INTO Trainer (Trainer_id, Age, Phone) VALUES
(101, 32, '555-0201'), -- Alice Williams
(102, 45, '555-0202'); -- Bob Johnson

PRINT 'Inserting into Worker (specialization of Employee)...';
INSERT INTO Worker (Worker_id, WorkerType, Age) VALUES
(201, 'Kennel Staff', 25), -- Charlie Brown
(202, 'Grooming Staff', 28); -- Diana Prince

-- *** CORRECTED: Must enable IDENTITY_INSERT for the Animal table ***
PRINT 'Inserting into Animal...';
SET IDENTITY_INSERT Animal ON;
INSERT INTO Animal (Animal_id, AnimalType, CNIC, Name, Age, StartDate, ReturnDate) VALUES
(1, 'Dog', '11111-1111111-1', 'Buddy', 3, '2023-01-15', '2023-02-15'), -- John Doe's Dog
(2, 'Cat', '22222-2222222-2', 'Whiskers', 5, '2023-03-01', '2023-03-10'), -- Jane Smith's Cat
(3, 'Dog', '33333-3333333-3', 'Rocky', 2, '2023-04-20', '2023-05-20'); -- Peter Jones' Dog
SET IDENTITY_INSERT Animal OFF;


-- Step 3: Populate the linking (many-to-many) tables
-----------------------------------------------------------------------------------

-- *** CORRECTED: Must enable IDENTITY_INSERT for the Training_Trainer table ***
PRINT 'Inserting into Training_Trainer (linking Trainers to Trainings)...';
SET IDENTITY_INSERT Training_Trainer ON;
INSERT INTO Training_Trainer (TT_id, Training_id, Trainer_id) VALUES
(1001, 1, 101), -- Alice teaches Basic Obedience
(1002, 4, 101), -- Alice also teaches Advanced Tricks
(1003, 2, 102), -- Bob teaches Agility Course
(1004, 3, 102); -- Bob also teaches Guard Dog Training
SET IDENTITY_INSERT Training_Trainer OFF;


-- NOTE: Animal_Training_Trainer does NOT have an IDENTITY column, so it's fine.
PRINT 'Inserting into Animal_Training_Trainer (assigning animals to a specific training session)...';
INSERT INTO Animal_Training_Trainer (Animal_id, TT_id) VALUES
(1, 1001), -- Assign Buddy (Dog) to Alice's Basic Obedience training
(3, 1004), -- Assign Rocky (Dog) to Bob's Guard Dog training
(3, 1003); -- Also assign Rocky (Dog) to Bob's Agility Course training
GO

-- =================================================================================
-- Verification Step: Select all data to confirm insertion
-- =================================================================================

PRINT '--- Verification ---';
SELECT * FROM Customer;
SELECT * FROM Allowed_Animal;
SELECT * FROM Employee;
SELECT * FROM Trainer;
SELECT * FROM Worker;
SELECT * FROM Training;						
SELECT * FROM Animal;
SELECT * FROM Training_Trainer;
SELECT * FROM Animal_Training_Trainer;
GO