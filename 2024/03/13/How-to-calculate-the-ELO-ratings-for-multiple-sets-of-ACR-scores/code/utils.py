"""
    Authors: wangwei1237
    Date:    2024/03/13 10:00
    Desc:    计算 ELO 得分中用到的相关工具函数.
"""

from scipy.special import comb
from tqdm import tqdm
from typing import List, Tuple, Callable  

import matplotlib.pyplot as plt
import numpy as np
import random

from elo import EloScore

def compute_elo_absoult_diff(ad: int, ratingA: int = 1500, ratingB: int = 1500) -> List[int]:
    """
        计算 ELO 得分.
        :param ad: A VS B 的 打分绝对差值，scoreA - scoreB.
        :param INIT_RATING_A: A 的初始 ELO 得分.
        :param INIT_RATING_B: B 的初始 ELO 得分.
        :return: 返回每一轮的 ELO 得分.
    """
    eloscore = EloScore(ratingA, ratingB)
    actualResults = eloscore.computeScoreFromAbsoultDiff(ad)
    eloscore.computeEloScore(actualResults)
    #print(ratingA, ratingB, eloscore.ratingA, eloscore.ratingB)
    return eloscore.ratingA, eloscore.ratingB


def compute_elo_ccr(mos: int, ratingA: int = 1500, ratingB: int = 1500) -> List[int]:
    """
        计算 ELO 得分.
        :param mos: A VS B 的 CCR 百分制打分.
        :param INIT_RATING_A: A 的初始 ELO 得分.
        :param INIT_RATING_B: B 的初始 ELO 得分.
        :return: 返回每一轮的 ELO 得分.
    """
    eloscore = EloScore(ratingA, ratingB)
    actualResults = eloscore.computeScoreFromPairMos(mos)
    eloscore.computeEloScore(actualResults)
    return eloscore.ratingA, eloscore.ratingB


def compute_elo_acr(mosA: int, mosB: int, ratingA: int = 1500, ratingB: int = 1500) -> List[int]:
    """
        计算 ELO 得分.
        :param mosA: A 的 ACR 打分.
        :param mosB: B 的 ACR 打分.
        :param INIT_RATING_A: A 的初始 ELO 得分.
        :param INIT_RATING_B: B 的初始 ELO 得分.
        :return: 返回每一轮的 ELO 得分.
    """
    eloscore = EloScore(ratingA, ratingB)
    actualResults = eloscore.computeScoreFromMos(mosA, mosB)
    eloscore.computeEloScore(actualResults)
    return eloscore.ratingA, eloscore.ratingB


def getAcrPairMos(fileMosA: str, fileMosB: str) -> Tuple[int, int]:
    """
        获取 A 和 B 两个文件中的所有 ACR 打分数据.
        :param fileMosA: A 的 ACR 打分记录.
        :param fileMosB: B 的 ACR 打分记录.
        :return: 返回 A 和 B 两个文件中的所有各自 ACR 打分数据.
    """
    with open(fileMosA, 'r') as fA, open(fileMosB, 'r') as fB:
        mosA = [int(line) for line in fA]
        mosB = [int(line) for line in fB]
    
    return mosA, mosB


def getCcrMos(fileMos: str) -> Tuple[int]:
    """
        获取 A VS B 的 MOS 打分数据.
        :param fileMos: 存放文件.
        :return: 返回 MOS 打分.
    """
    with open(fileMos, 'r') as f: 
        mos = [int(line) for line in f]
    
    return mos


def get_bootstrap_ccr_result(mos: List[int], 
                             func_compute_elo: Callable[[int, int, int], List[int]], 
                             num_round: int, 
                             init_A: int = 1500, 
                             init_B: int = 1500) -> Tuple[int, int]:
    """ 
        获取 A 和 B 的所有 CCR 打分数据计算最终的 ELO 得分.
        :param mos: A VS B 的 CCR 打分记录.
        :func_compute_elo: 计算 ELO 得分函数.
        :num_round: bootstrap 计算轮数.
        :return: 返回 A 和 B 每一轮打分的 ELO 得分.
    """
    # 计算 ELO 得分.
    ratingA    = init_A  # 初始 ELO 得分.
    ratingB    = init_B  # 初始 ELO 得分.
    eloA = []
    eloB = []
    for i in tqdm(range(num_round), desc="bootstrap"):
        random.shuffle(mos)
        for m in mos:
            ratingA, ratingB = func_compute_elo(m, ratingA, ratingB)
        eloA.append(ratingA)
        eloB.append(ratingB)
    
    return eloA, eloB


def get_bootstrap_acr_result(mosA: List[int], 
                             mosB: List[int], 
                             func_compute_elo: Callable[[int, int, int, int], List[int]], 
                             num_round: int, 
                             init_A: int = 1500, 
                             init_B: int = 1500) -> Tuple[int, int]:
    """ 
        根据 A 和 B 的所有 ACR 打分数据计算最终的 ELO 得分.
        :param mosA: A 的 ACR 打分记录.
        :param mosB: B 的 ACR 打分记录.
        :func_compute_elo: 计算 ELO 得分函数.
        :num_round: bootstrap 计算轮数.
        :return: 返回 A 和 B 每一轮打分的 ELO 得分.
    """
    # 计算 ELO 得分.
    batch_size = int(len(mosA) / 5) # 每次计算 batch_size 个 ACR 打分.
    ratingA    = init_A  # 初始 ELO 得分.
    ratingB    = init_B  # 初始 ELO 得分.
    eloA = []
    eloB = []
    for i in tqdm(range(num_round), desc="bootstrap"):
        A = random.sample(mosA, batch_size)
        B = random.sample(mosB, batch_size)
        meanA = int(np.mean(A))
        meanB = int(np.mean(B))
        ratingA, ratingB = func_compute_elo(meanA, meanB, ratingA, ratingB)
        eloA.append(ratingA)
        eloB.append(ratingB)
    
    return eloA, eloB


def visual_bootstrap_scores(eloA: List[int], eloB: List[int]) -> None:
    """ 
        可视化 bootstrap 计算结果.
        :param eloA: A 的 ELO 得分.
        :param eloB: B 的 ELO 得分.
    """
    assert len(eloA) == len(eloB), 'len of A and B must be equal.'

    # 每轮得分趋势
    plt.subplot(1, 2, 1)
    X = np.arange(len(eloA))
    #plt.scatter(X, eloA, marker='x', label='A')
    #plt.scatter(X, eloB, marker='o', label='B')
    plt.plot(X, eloA, linestyle='--', color='red', label='A')
    plt.plot(X, eloB, linestyle='-', color='blue', label='B')
    plt.legend(loc="best")
    plt.title('A vs B')

    # 每轮打分分布
    plt.subplot(1, 2, 2)
    
    X = np.linspace(min(np.min(eloA), np.min(eloB)), max(np.max(eloA), np.max(eloB)), 100)
    mu = np.mean(eloA)
    sigma = np.std(eloA)
    plt.plot(X, 1 / (sigma * np.sqrt(2 * np.pi)) * np.exp(-(X - mu) ** 2 / (2 * sigma ** 2)),
        linewidth=2, color='r', label=r"A: μ=%d,σ=%d" % (mu, sigma))
    plt.hist(eloA, bins=10, label="A", density=True, alpha=0.3)

    mu = np.mean(eloB)
    sigma = np.std(eloB)
    plt.plot(X, 1 / (sigma * np.sqrt(2 * np.pi)) * np.exp(-(X - mu) ** 2 / (2 * sigma ** 2)),
        linewidth=2, color='g', label=r"B: μ=%d,σ=%d" % (mu, sigma))
    plt.hist(eloB, bins=10, label="B", density=True, alpha=0.3)
    plt.legend(loc="best")
    plt.title('Distrubtion A VS B')
    
    
if __name__ == "__main__":
    pass