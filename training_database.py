"""SQLite persistence layer for the CS 370 Pirate Intelligent Agent.

CS 499 Milestone Four: Database Enhancement
Student: Arishia Jackson
"""
from __future__ import annotations

import math
import sqlite3
from contextlib import contextmanager
from datetime import datetime, timezone
from pathlib import Path
from typing import Iterator, Optional


class TrainingDatabase:
    """Stores training runs and epoch metrics using parameterized SQLite queries."""

    def __init__(self, database_path: str = "pirate_training.db") -> None:
        self.database_path = Path(database_path)
        self._initialize_schema()

    @contextmanager
    def _connection(self) -> Iterator[sqlite3.Connection]:
        connection = sqlite3.connect(self.database_path)
        connection.row_factory = sqlite3.Row
        connection.execute("PRAGMA foreign_keys = ON")
        try:
            yield connection
            connection.commit()
        except sqlite3.Error:
            connection.rollback()
            raise
        finally:
            connection.close()

    def _initialize_schema(self) -> None:
        schema = """
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

        CREATE INDEX IF NOT EXISTS idx_epoch_metrics_run_epoch
            ON epoch_metrics(run_id, epoch_number);
        CREATE INDEX IF NOT EXISTS idx_training_runs_started_at
            ON training_runs(started_at);
        """
        with self._connection() as connection:
            connection.executescript(schema)

    @staticmethod
    def _finite_number(value: float, name: str) -> float:
        numeric = float(value)
        if not math.isfinite(numeric):
            raise ValueError(f"{name} must be a finite number.")
        return numeric

    def start_run(self, *, maze_rows: int, maze_columns: int, requested_epochs: int,
                  max_memory: int, batch_size: int, target_update_frequency: int,
                  epsilon_start: float, epsilon_minimum: float, epsilon_decay: float) -> int:
        integer_values = {
            "maze_rows": maze_rows, "maze_columns": maze_columns,
            "requested_epochs": requested_epochs, "max_memory": max_memory,
            "batch_size": batch_size, "target_update_frequency": target_update_frequency,
        }
        for name, value in integer_values.items():
            if int(value) <= 0:
                raise ValueError(f"{name} must be greater than zero.")
        epsilon_start = self._finite_number(epsilon_start, "epsilon_start")
        epsilon_minimum = self._finite_number(epsilon_minimum, "epsilon_minimum")
        epsilon_decay = self._finite_number(epsilon_decay, "epsilon_decay")
        if not 0.0 <= epsilon_minimum <= epsilon_start <= 1.0:
            raise ValueError("Epsilon values must satisfy 0 <= minimum <= start <= 1.")
        if not 0.0 < epsilon_decay <= 1.0:
            raise ValueError("epsilon_decay must be greater than 0 and at most 1.")

        query = """INSERT INTO training_runs (
            started_at, status, maze_rows, maze_columns, requested_epochs,
            max_memory, batch_size, target_update_frequency,
            epsilon_start, epsilon_minimum, epsilon_decay
        ) VALUES (?, 'running', ?, ?, ?, ?, ?, ?, ?, ?, ?)"""
        values = (datetime.now(timezone.utc).isoformat(), int(maze_rows), int(maze_columns),
                  int(requested_epochs), int(max_memory), int(batch_size),
                  int(target_update_frequency), epsilon_start, epsilon_minimum, epsilon_decay)
        with self._connection() as connection:
            cursor = connection.execute(query, values)
            return int(cursor.lastrowid)

    def record_epoch(self, *, run_id: int, epoch_number: int, loss: float,
                     episode_steps: int, cumulative_wins: int, win_rate: float,
                     epsilon: float, elapsed_seconds: float) -> None:
        loss = self._finite_number(loss, "loss")
        win_rate = self._finite_number(win_rate, "win_rate")
        epsilon = self._finite_number(epsilon, "epsilon")
        elapsed_seconds = self._finite_number(elapsed_seconds, "elapsed_seconds")
        if epoch_number < 0 or episode_steps < 0 or cumulative_wins < 0:
            raise ValueError("Epoch counts and steps cannot be negative.")
        if not 0.0 <= win_rate <= 1.0 or not 0.0 <= epsilon <= 1.0:
            raise ValueError("win_rate and epsilon must be between 0 and 1.")
        if elapsed_seconds < 0.0:
            raise ValueError("elapsed_seconds cannot be negative.")

        query = """INSERT INTO epoch_metrics (
            run_id, epoch_number, loss, episode_steps, cumulative_wins,
            win_rate, epsilon, elapsed_seconds, recorded_at
        ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)
        ON CONFLICT(run_id, epoch_number) DO UPDATE SET
            loss=excluded.loss, episode_steps=excluded.episode_steps,
            cumulative_wins=excluded.cumulative_wins, win_rate=excluded.win_rate,
            epsilon=excluded.epsilon, elapsed_seconds=excluded.elapsed_seconds,
            recorded_at=excluded.recorded_at"""
        values = (int(run_id), int(epoch_number), loss, int(episode_steps),
                  int(cumulative_wins), win_rate, epsilon, elapsed_seconds,
                  datetime.now(timezone.utc).isoformat())
        with self._connection() as connection:
            connection.execute(query, values)

    def finish_run(self, run_id: int, *, status: str, final_epoch: int,
                   final_win_rate: float, total_seconds: float) -> None:
        if status not in {"completed", "failed"}:
            raise ValueError("status must be 'completed' or 'failed'.")
        final_win_rate = self._finite_number(final_win_rate, "final_win_rate")
        total_seconds = self._finite_number(total_seconds, "total_seconds")
        if final_epoch < 0 or not 0.0 <= final_win_rate <= 1.0 or total_seconds < 0:
            raise ValueError("Invalid final training values.")
        query = """UPDATE training_runs SET completed_at=?, status=?, final_epoch=?,
                   final_win_rate=?, total_seconds=? WHERE run_id=?"""
        with self._connection() as connection:
            cursor = connection.execute(query, (datetime.now(timezone.utc).isoformat(), status,
                                                 int(final_epoch), final_win_rate,
                                                 total_seconds, int(run_id)))
            if cursor.rowcount != 1:
                raise KeyError(f"Training run {run_id} does not exist.")

    def recent_runs(self, limit: int = 10) -> list[dict]:
        if not 1 <= int(limit) <= 100:
            raise ValueError("limit must be between 1 and 100.")
        query = """SELECT run_id, started_at, completed_at, status, requested_epochs,
                   final_epoch, final_win_rate, total_seconds
                   FROM training_runs ORDER BY run_id DESC LIMIT ?"""
        with self._connection() as connection:
            return [dict(row) for row in connection.execute(query, (int(limit),)).fetchall()]

    def run_metrics(self, run_id: int) -> list[dict]:
        query = """SELECT epoch_number, loss, episode_steps, cumulative_wins,
                   win_rate, epsilon, elapsed_seconds
                   FROM epoch_metrics WHERE run_id=? ORDER BY epoch_number"""
        with self._connection() as connection:
            return [dict(row) for row in connection.execute(query, (int(run_id),)).fetchall()]

    def delete_run(self, run_id: int) -> bool:
        with self._connection() as connection:
            cursor = connection.execute("DELETE FROM training_runs WHERE run_id=?", (int(run_id),))
            return cursor.rowcount == 1
