from pathlib import Path

import numpy as np
import torch
from torch import nn
from torch.utils.data import DataLoader, TensorDataset


BASE_DIR = Path(__file__).resolve().parent
DATASET_PATH = BASE_DIR / "neural_gi_dataset.npz"
OUTPUT_PATH = BASE_DIR / "neural_gi_weights.npz"


class NeuralGI(nn.Module):
    def __init__(self):
        super().__init__()
        self.net = nn.Sequential(
            nn.Linear(11, 16),
            nn.ReLU(),
            nn.Linear(16, 16),
            nn.ReLU(),
            nn.Linear(16, 3),
            nn.Sigmoid(),
        )

    def forward(self, x):
        return self.net(x)


def main():
    if not DATASET_PATH.exists():
        raise FileNotFoundError(f"Missing dataset: {DATASET_PATH}. Run generate_dataset.py first.")

    data = np.load(DATASET_PATH)
    x = torch.from_numpy(data["x"]).float()
    y = torch.from_numpy(data["y"]).float()

    loader = DataLoader(TensorDataset(x, y), batch_size=256, shuffle=True)
    model = NeuralGI()
    optimizer = torch.optim.Adam(model.parameters(), lr=1e-3)
    loss_fn = nn.MSELoss()

    for epoch in range(1, 81):
        total_loss = 0.0
        for batch_x, batch_y in loader:
            optimizer.zero_grad()
            pred = model(batch_x)
            loss = loss_fn(pred, batch_y)
            loss.backward()
            optimizer.step()
            total_loss += loss.item() * batch_x.size(0)

        if epoch % 10 == 0 or epoch == 1:
            print(f"epoch {epoch:03d} | mse {total_loss / len(loader.dataset):.8f}")

    state = model.state_dict()
    np.savez(
        OUTPUT_PATH,
        w1=state["net.0.weight"].numpy(),
        b1=state["net.0.bias"].numpy(),
        w2=state["net.2.weight"].numpy(),
        b2=state["net.2.bias"].numpy(),
        w3=state["net.4.weight"].numpy(),
        b3=state["net.4.bias"].numpy(),
    )
    print(f"Saved weights to {OUTPUT_PATH}")


if __name__ == "__main__":
    main()
