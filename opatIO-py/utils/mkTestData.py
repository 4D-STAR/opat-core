from opatio import OpatIO
import numpy as np
np.random.seed(42)

def generate_synthetic_opacity_table(n_r, n_t):
    logR = np.linspace(-8, 2, n_r, dtype=np.float64)  # log Density grid
    logT = np.linspace(3, 9, n_t, dtype=np.float64)  # log Temperature grid
    logK = np.random.uniform(-2, 2, size=(n_r, n_t)).astype(np.float64)  # Synthetic Opacity
    return logR, logT, logK

if __name__ == "__main__":
    n_r = 50
    n_t = 50
    num_tables = 20
    XValues = np.linspace(0.1, 0.7, num_tables)
    ZValues = np.linspace(0.001, 0.03, num_tables)
    opat = OpatIO()
    opat.set_comment("Synthetic Opacity Tables")
    opat.set_source("utils/opatio/utils/mkTestData.py")

    for i in range(num_tables):
        logR, logT, logK = generate_synthetic_opacity_table(n_r, n_t)
        opat.add_table((XValues[i], ZValues[i]), logR, logT, logK)

    opat.save("testData/synthetic_tables.opat")
    opat.save_as_ascii("testData/synthetic_tables_OPAT.ascii")

