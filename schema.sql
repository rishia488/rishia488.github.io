-- CS 499 Milestone Four: Pirate training database schema
PRAGMA foreign_keys = ON;

CREATE TABLE IF NOT EXISTS training_runs (
    run_id INTEGER PRIMARY KEY AUTOINCREMENT,
    started_at TEXT NOT NULL,
    completed_at TEXT,
    status TEXT NOT NULL CHECK(status IN ('running', 'completed', 'failed')),
    maze_rows INTEGER NOT NULL CHECK(maze_rows > 0),
    maze_columns INTEGER NOT NULL CHECK(maze_columns > 0),
    requested_epochs INTEGER NOT NULL CHECK(requested_epochs > 0),
    max_memory INTEGER NOT NULL CHECK(max_memory > 0),
    batch_size INTEGER NOT NULL CHECK(batch_size > 0),
    target_update_frequency INTEGER NOT NULL CHECK(target_update_frequency > 0),
    epsilon_start REAL NOT NULL CHECK(epsilon_start BETWEEN 0.0 AND 1.0),
    epsilon_minimum REAL NOT NULL CHECK(epsilon_minimum BETWEEN 0.0 AND 1.0),
    epsilon_decay REAL NOT NULL CHECK(epsilon_decay > 0.0 AND epsilon_decay <= 1.0),
    final_epoch INTEGER,
    final_win_rate REAL CHECK(final_win_rate BETWEEN 0.0 AND 1.0),
    total_seconds REAL CHECK(total_seconds >= 0.0)
);

CREATE TABLE IF NOT EXISTS epoch_metrics (
    metric_id INTEGER PRIMARY KEY AUTOINCREMENT,
    run_id INTEGER NOT NULL,
    epoch_number INTEGER NOT NULL CHECK(epoch_number >= 0),
    loss REAL NOT NULL,
    episode_steps INTEGER NOT NULL CHECK(episode_steps >= 0),
    cumulative_wins INTEGER NOT NULL CHECK(cumulative_wins >= 0),
    win_rate REAL NOT NULL CHECK(win_rate BETWEEN 0.0 AND 1.0),
    epsilon REAL NOT NULL CHECK(epsilon BETWEEN 0.0 AND 1.0),
    elapsed_seconds REAL NOT NULL CHECK(elapsed_seconds >= 0.0),
    recorded_at TEXT NOT NULL,
    FOREIGN KEY (run_id) REFERENCES training_runs(run_id) ON DELETE CASCADE,
    UNIQUE(run_id, epoch_number)
);
CREATE INDEX IF NOT EXISTS idx_epoch_metrics_run_epoch ON epoch_metrics(run_id, epoch_number);
CREATE INDEX IF NOT EXISTS idx_training_runs_started_at ON training_runs(started_at);
