#encoding=utf-8

"""
    Authors: wangwei17
    Date:    2023/08/07 15:33
    Desc:    Elo得分
"""
class EloScore:
    """
    计算Elo得分
    """

    ELO_RESULT_WIN  = 1
    ELO_RESULT_TIE  = 0.5
    ELO_RESULT_LOSS = 0

    ELO_RATING_DEFAULT = 1500

    ratingA = 0
    ratingB = 0

    def __init__(self, ratingA=ELO_RATING_DEFAULT, ratingB=ELO_RATING_DEFAULT):
        """
        初始化函数，设置对弈双方的初始 ELO 评级。
        """
        self.ratingA = ratingA
        self.ratingB = ratingB

    def computeK(self, rating):
        """
        根据评分计算K值
        """
        if rating >= 2400:
            return 16
        elif rating >= 2000:
            return 24
        else:
            return 32

    def computeExpectScore(self, ratingA, ratingB):
        """
        计算预期得分。
        """
        return 1 / (1 + pow(10, (ratingB - ratingA) / 400))

    def computeEloScore(self, actualResults=ELO_RESULT_TIE):
        """
        计算 Elo 评分
        """
        expectScore = self.computeExpectScore(self.ratingA, self.ratingB)
        
        kA = self.computeK(self.ratingA)
        kB = self.computeK(self.ratingB)

        scoresA = round(kA * (actualResults - expectScore))
        scoresB = round(kB * ((1 - actualResults) - (1 - expectScore)))
        self.ratingA += scoresA
        self.ratingB += scoresB
    
    def computeScoreFromMos(self, mos):
        """
        根据MOS值计算得分。
        """
        if mos >= 60:
            return self.ELO_RESULT_LOSS
        elif mos >= 40:
            return self.ELO_RESULT_TIE
        else: 
            return self.ELO_RESULT_WIN

import matplotlib.pyplot as plt
import numpy as np

def eloDiffScoreDistribution():
    """
    计算两个 Elo 得分差的预期胜算概率分布
    """
    X   = np.linspace(-1000, 1000, 50)
    res = []
    for d in X:
        res.append(1 / (1 + pow(10, -d / 400)))
    
    plt.grid(True)
    plt.title('P(D) curve of change with D')
    plt.plot(X, res, linewidth=3, color='g')

def eloScoreDistribution():
    """
    计算 Elo 实际得分与分差的变化趋势
    """
    K   = 32
    X   = np.linspace(-1000, 1000, 50)
    res = []
    for d in X:
        res.append(K * (1 - 1 / (1 + pow(10, -d / 400))))
    
    plt.grid(True)
    plt.title('Elo increasing scores curve of change with D')
    plt.plot(X, res, linewidth=3, color='g')

def getEloScores():
    """
    根据视频的MOS分计算编解码器的Elo得分
    """
    eloscore = EloScore(1500, 1500)
    A = []
    B = []
    X = []

    lines = open("MOS").readlines()
    for mos in lines:
        mos = int(mos.rstrip())
        actualResults = eloscore.computeScoreFromMos(mos)
        eloscore.computeEloScore(actualResults)
        A.append(eloscore.ratingA)
        B.append(eloscore.ratingB)
        X.append(len(X) + 1)

    print(eloscore.ratingA, "\t", eloscore.ratingB)
    
    plt.scatter(X, A, marker='x', label='Codec-A')
    plt.scatter(X, B, marker='o', label='Codec-B')
    plt.legend(loc="best")
    plt.title('Codec-A vs Codec-B')

def getTwoNormalDistribution():
    """
    获取两个正态分布的变化趋势
    """
    x = np.linspace(1000, 2600, 100)
    mu = 1800
    sigma = 200
    plt.plot(x, 1/(sigma * np.sqrt(2 * np.pi)) * np.exp(- (x - mu)**2 / (2 * sigma**2)),
        linewidth=2, color='r', label=r"A: μ=1800,σ=200")

    mu = 2000
    sigma = 100
    plt.plot(x, 1/(sigma * np.sqrt(2 * np.pi)) * np.exp(- (x - mu)**2 / (2 * sigma**2)),
        linewidth=2, color='g', label=r"B: μ=2000,σ=100")
    plt.legend(loc="best")

def getNormalDistribution():
    """
    获取正态分布的动态变化趋势
    """
    x = np.linspace(1000, 2600, 100)
   
    plt.ion()
    line_y = np.arange(0, 0.004, 0.0001)
    i = 1000
    while i <= 2600:
        plt.cla()
        mu = 1800
        sigma = 200
        plt.plot(x, 1/(sigma * np.sqrt(2 * np.pi)) * np.exp(- (x - mu)**2 / (2 * sigma**2)),
            linewidth=2, color='r', label=r"μ=1800,σ=200")
    
        mu = 2000
        sigma = 100
        plt.plot(x, 1/(sigma * np.sqrt(2 * np.pi)) * np.exp(- (x - mu)**2 / (2 * sigma**2)),
            linewidth=2, color='g', label=r"μ=2000,σ=100")
        
        line_x = np.ones_like(line_y) * i
        plt.plot(line_x, line_y, label=str(i))
        plt.legend(loc="upper left")
        plt.pause(0.001)
        i += 50
        if i > 2600:
            i = 1000
    plt.ioff()

if __name__ == "__main__":
    # eloDiffScoreDistribution()
    # eloScoreDistribution()
    getEloScores()
    # getTwoNormalDistribution()
    # getNormalDistribution()
    plt.show()
