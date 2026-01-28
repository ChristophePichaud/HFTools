-- Sybase ASE DDL for Financial System
-- Database: hftools_db

-- Create database (run this separately as sa)
-- CREATE DATABASE hftools_db
-- GO
-- USE hftools_db
-- GO

-- Users table
IF NOT EXISTS (SELECT * FROM sysobjects WHERE name='users' AND type='U')
BEGIN
    CREATE TABLE users (
        id INT IDENTITY(1,1) PRIMARY KEY,
        username VARCHAR(50) NOT NULL UNIQUE,
        email VARCHAR(100) NOT NULL UNIQUE,
        role VARCHAR(20) NOT NULL CHECK (role IN ('TRADER', 'ADMIN', 'ANALYST', 'MANAGER')),
        created_at DATETIME DEFAULT GETDATE(),
        updated_at DATETIME DEFAULT GETDATE()
    )
END
GO

-- FX Instruments table
IF NOT EXISTS (SELECT * FROM sysobjects WHERE name='fxinstruments' AND type='U')
BEGIN
    CREATE TABLE fxinstruments (
        id INT IDENTITY(1,1) PRIMARY KEY,
        symbol VARCHAR(10) NOT NULL UNIQUE,
        base_currency VARCHAR(3) NOT NULL,
        quote_currency VARCHAR(3) NOT NULL,
        tick_size DECIMAL(10, 6) NOT NULL DEFAULT 0.0001,
        created_at DATETIME DEFAULT GETDATE(),
        updated_at DATETIME DEFAULT GETDATE()
    )
END
GO

-- Trades table
IF NOT EXISTS (SELECT * FROM sysobjects WHERE name='trades' AND type='U')
BEGIN
    CREATE TABLE trades (
        id INT IDENTITY(1,1) PRIMARY KEY,
        user_id INT NOT NULL,
        instrument_id INT NOT NULL,
        side VARCHAR(4) NOT NULL CHECK (side IN ('BUY', 'SELL')),
        quantity DECIMAL(18, 4) NOT NULL,
        price DECIMAL(18, 6) NOT NULL,
        timestamp DATETIME NOT NULL DEFAULT GETDATE(),
        created_at DATETIME DEFAULT GETDATE(),
        FOREIGN KEY (user_id) REFERENCES users(id),
        FOREIGN KEY (instrument_id) REFERENCES fxinstruments(id)
    )
END
GO

-- Create indexes for better query performance
CREATE INDEX idx_trades_user_id ON trades(user_id)
GO
CREATE INDEX idx_trades_instrument_id ON trades(instrument_id)
GO
CREATE INDEX idx_trades_timestamp ON trades(timestamp)
GO
CREATE INDEX idx_users_username ON users(username)
GO
CREATE INDEX idx_fxinstruments_symbol ON fxinstruments(symbol)
GO

-- Insert sample data

-- Sample users
IF NOT EXISTS (SELECT * FROM users WHERE username='trader1')
    INSERT INTO users (username, email, role) VALUES ('trader1', 'trader1@example.com', 'TRADER')
GO
IF NOT EXISTS (SELECT * FROM users WHERE username='admin1')
    INSERT INTO users (username, email, role) VALUES ('admin1', 'admin1@example.com', 'ADMIN')
GO
IF NOT EXISTS (SELECT * FROM users WHERE username='trader2')
    INSERT INTO users (username, email, role) VALUES ('trader2', 'trader2@example.com', 'TRADER')
GO
IF NOT EXISTS (SELECT * FROM users WHERE username='analyst1')
    INSERT INTO users (username, email, role) VALUES ('analyst1', 'analyst1@example.com', 'ANALYST')
GO
IF NOT EXISTS (SELECT * FROM users WHERE username='manager1')
    INSERT INTO users (username, email, role) VALUES ('manager1', 'manager1@example.com', 'MANAGER')
GO

-- Sample FX instruments
IF NOT EXISTS (SELECT * FROM fxinstruments WHERE symbol='EUR/USD')
    INSERT INTO fxinstruments (symbol, base_currency, quote_currency, tick_size) VALUES ('EUR/USD', 'EUR', 'USD', 0.0001)
GO
IF NOT EXISTS (SELECT * FROM fxinstruments WHERE symbol='GBP/USD')
    INSERT INTO fxinstruments (symbol, base_currency, quote_currency, tick_size) VALUES ('GBP/USD', 'GBP', 'USD', 0.0001)
GO
IF NOT EXISTS (SELECT * FROM fxinstruments WHERE symbol='USD/JPY')
    INSERT INTO fxinstruments (symbol, base_currency, quote_currency, tick_size) VALUES ('USD/JPY', 'USD', 'JPY', 0.01)
GO
IF NOT EXISTS (SELECT * FROM fxinstruments WHERE symbol='USD/CHF')
    INSERT INTO fxinstruments (symbol, base_currency, quote_currency, tick_size) VALUES ('USD/CHF', 'USD', 'CHF', 0.0001)
GO
IF NOT EXISTS (SELECT * FROM fxinstruments WHERE symbol='AUD/USD')
    INSERT INTO fxinstruments (symbol, base_currency, quote_currency, tick_size) VALUES ('AUD/USD', 'AUD', 'USD', 0.0001)
GO
IF NOT EXISTS (SELECT * FROM fxinstruments WHERE symbol='USD/CAD')
    INSERT INTO fxinstruments (symbol, base_currency, quote_currency, tick_size) VALUES ('USD/CAD', 'USD', 'CAD', 0.0001)
GO
IF NOT EXISTS (SELECT * FROM fxinstruments WHERE symbol='NZD/USD')
    INSERT INTO fxinstruments (symbol, base_currency, quote_currency, tick_size) VALUES ('NZD/USD', 'NZD', 'USD', 0.0001)
GO
IF NOT EXISTS (SELECT * FROM fxinstruments WHERE symbol='EUR/GBP')
    INSERT INTO fxinstruments (symbol, base_currency, quote_currency, tick_size) VALUES ('EUR/GBP', 'EUR', 'GBP', 0.0001)
GO
IF NOT EXISTS (SELECT * FROM fxinstruments WHERE symbol='EUR/JPY')
    INSERT INTO fxinstruments (symbol, base_currency, quote_currency, tick_size) VALUES ('EUR/JPY', 'EUR', 'JPY', 0.01)
GO
IF NOT EXISTS (SELECT * FROM fxinstruments WHERE symbol='GBP/JPY')
    INSERT INTO fxinstruments (symbol, base_currency, quote_currency, tick_size) VALUES ('GBP/JPY', 'GBP', 'JPY', 0.01)
GO

-- Sample trades (check if trades table is empty first)
IF NOT EXISTS (SELECT * FROM trades)
BEGIN
    INSERT INTO trades (user_id, instrument_id, side, quantity, price, timestamp) VALUES
        (1, 1, 'BUY', 100000.00, 1.0850, '2024-01-28 10:30:00')
    INSERT INTO trades (user_id, instrument_id, side, quantity, price, timestamp) VALUES
        (1, 2, 'SELL', 50000.00, 1.2675, '2024-01-28 11:15:00')
    INSERT INTO trades (user_id, instrument_id, side, quantity, price, timestamp) VALUES
        (3, 3, 'BUY', 200000.00, 149.85, '2024-01-28 12:00:00')
    INSERT INTO trades (user_id, instrument_id, side, quantity, price, timestamp) VALUES
        (3, 1, 'BUY', 75000.00, 1.0855, '2024-01-28 14:20:00')
    INSERT INTO trades (user_id, instrument_id, side, quantity, price, timestamp) VALUES
        (1, 4, 'SELL', 100000.00, 0.8765, '2024-01-28 15:45:00')
    INSERT INTO trades (user_id, instrument_id, side, quantity, price, timestamp) VALUES
        (3, 5, 'BUY', 150000.00, 0.6543, '2024-01-28 16:10:00')
    INSERT INTO trades (user_id, instrument_id, side, quantity, price, timestamp) VALUES
        (1, 2, 'BUY', 80000.00, 1.2680, '2024-01-28 16:45:00')
    INSERT INTO trades (user_id, instrument_id, side, quantity, price, timestamp) VALUES
        (3, 3, 'SELL', 100000.00, 149.90, '2024-01-28 17:20:00')
END
GO
