from pathlib import Path
from training_database import TrainingDatabase

path=Path('database_test.db')
if path.exists(): path.unlink()
db=TrainingDatabase(str(path))
run=db.start_run(maze_rows=8, maze_columns=8, requested_epochs=1000,
                 max_memory=512, batch_size=32, target_update_frequency=50,
                 epsilon_start=1.0, epsilon_minimum=0.01, epsilon_decay=0.995)
db.record_epoch(run_id=run, epoch_number=0, loss=0.75, episode_steps=20,
                cumulative_wins=0, win_rate=0.0, epsilon=1.0, elapsed_seconds=1.25)
db.record_epoch(run_id=run, epoch_number=1, loss=0.50, episode_steps=18,
                cumulative_wins=1, win_rate=0.5, epsilon=0.995, elapsed_seconds=2.50)
db.finish_run(run, status='completed', final_epoch=1, final_win_rate=0.5, total_seconds=2.5)
print('RECENT RUNS')
print(db.recent_runs())
print('RUN METRICS')
print(db.run_metrics(run))
print('DELETE:', db.delete_run(run))
print('AFTER DELETE:', db.recent_runs())
path.unlink()
