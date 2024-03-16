"""
    Authors: wangwei1237
    Date:    2024/03/07 15:33
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
    
    def computeScoreFromPairMos(self, mos: int):
        """
        根据成对打分的MOS值计算得分。
        """
        if mos >= 60:
            return self.ELO_RESULT_LOSS
        elif mos >= 40:
            return self.ELO_RESULT_TIE
        else: 
            return self.ELO_RESULT_WIN
    
    def computeScoreFromMos(self, mosA: int, mosB: int):
        """
        根据 ACR 打分的MOS值计算得分。
        """
        if mosA < mosB:
            return self.ELO_RESULT_LOSS
        elif mosA == mosB:
            return self.ELO_RESULT_TIE
        else: 
            return self.ELO_RESULT_WIN
    
    def computeScoreFromAbsoultDiff(self, res: int):
        """
        根据双方的差值打分计算得分。
        """
        if res < 0:
            return self.ELO_RESULT_LOSS
        elif res == 0:
            return self.ELO_RESULT_TIE
        else: 
            return self.ELO_RESULT_WIN
