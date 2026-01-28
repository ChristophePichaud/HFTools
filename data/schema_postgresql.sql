-- PostgreSQL DDL for Financial System
-- Database: hftools_db

-- Create database (run this separately as superuser)
-- CREATE DATABASE hftools_db;

-- Connect to the database
-- \c hftools_db

-- Users table
CREATE TABLE IF NOT EXISTS users (
    id SERIAL PRIMARY KEY,
    username VARCHAR(50) NOT NULL UNIQUE,
    email VARCHAR(100) NOT NULL UNIQUE,
    role VARCHAR(20) NOT NULL CHECK (role IN ('TRADER', 'ADMIN', 'ANALYST', 'MANAGER')),
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- FX Instruments table
CREATE TABLE IF NOT EXISTS fxinstruments (
    id SERIAL PRIMARY KEY,
    symbol VARCHAR(10) NOT NULL UNIQUE,
    base_currency VARCHAR(3) NOT NULL,
    quote_currency VARCHAR(3) NOT NULL,
    tick_size DECIMAL(10, 6) NOT NULL DEFAULT 0.0001,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Trades table
CREATE TABLE IF NOT EXISTS trades (
    id SERIAL PRIMARY KEY,
    user_id INTEGER NOT NULL,
    instrument_id INTEGER NOT NULL,
    side VARCHAR(4) NOT NULL CHECK (side IN ('BUY', 'SELL')),
    quantity DECIMAL(18, 4) NOT NULL,
    price DECIMAL(18, 6) NOT NULL,
    timestamp TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES users(id),
    FOREIGN KEY (instrument_id) REFERENCES fxinstruments(id)
);

-- Create indexes for better query performance
CREATE INDEX IF NOT EXISTS idx_trades_user_id ON trades(user_id);
CREATE INDEX IF NOT EXISTS idx_trades_instrument_id ON trades(instrument_id);
CREATE INDEX IF NOT EXISTS idx_trades_timestamp ON trades(timestamp);
CREATE INDEX IF NOT EXISTS idx_users_username ON users(username);
CREATE INDEX IF NOT EXISTS idx_fxinstruments_symbol ON fxinstruments(symbol);

-- Insert sample data

-- Sample users
INSERT INTO users (username, email, role) VALUES
    ('trader1', 'trader1@example.com', 'TRADER'),
    ('admin1', 'admin1@example.com', 'ADMIN'),
    ('trader2', 'trader2@example.com', 'TRADER'),
    ('analyst1', 'analyst1@example.com', 'ANALYST'),
    ('manager1', 'manager1@example.com', 'MANAGER')
ON CONFLICT (username) DO NOTHING;

-- Sample FX instruments
INSERT INTO fxinstruments (symbol, base_currency, quote_currency, tick_size) VALUES
    ('EUR/USD', 'EUR', 'USD', 0.0001),
    ('GBP/USD', 'GBP', 'USD', 0.0001),
    ('USD/JPY', 'USD', 'JPY', 0.01),
    ('USD/CHF', 'USD', 'CHF', 0.0001),
    ('AUD/USD', 'AUD', 'USD', 0.0001),
    ('USD/CAD', 'USD', 'CAD', 0.0001),
    ('NZD/USD', 'NZD', 'USD', 0.0001),
    ('EUR/GBP', 'EUR', 'GBP', 0.0001),
    ('EUR/JPY', 'EUR', 'JPY', 0.01),
    ('GBP/JPY', 'GBP', 'JPY', 0.01)
ON CONFLICT (symbol) DO NOTHING;

-- Sample trades
INSERT INTO trades (user_id, instrument_id, side, quantity, price, timestamp) VALUES
    (1, 1, 'BUY', 100000.00, 1.0850, '2024-01-28 10:30:00'),
    (1, 2, 'SELL', 50000.00, 1.2675, '2024-01-28 11:15:00'),
    (3, 3, 'BUY', 200000.00, 149.85, '2024-01-28 12:00:00'),
    (3, 1, 'BUY', 75000.00, 1.0855, '2024-01-28 14:20:00'),
    (1, 4, 'SELL', 100000.00, 0.8765, '2024-01-28 15:45:00'),
    (3, 5, 'BUY', 150000.00, 0.6543, '2024-01-28 16:10:00'),
    (1, 2, 'BUY', 80000.00, 1.2680, '2024-01-28 16:45:00'),
    (3, 3, 'SELL', 100000.00, 149.90, '2024-01-28 17:20:00');
