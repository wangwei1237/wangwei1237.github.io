"""
    Authors: wangwei1237
    Date:    2024/03/13 10:00
    Desc:    测试根据 ACR 计算 ELO.
"""

import matplotlib.pyplot as plt
import numpy as np
import utils

mosA = "mosA"
mosB = "mosB"
mosA, mosB = utils.getAcrPairMos(mosA, mosB)
A, B = utils.get_bootstrap_acr_result(mosA, mosB, utils.compute_elo_acr, 5000)
utils.visual_bootstrap_scores(A, B)
plt.show()