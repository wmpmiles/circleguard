import numpy as np

from replay import Replay

class Investigator:
    """
    A class for checking isolated replays for cheats. 

    See Also:
        Comparer
    """
    def __init__(self):
        pass

    @staticmethod
    def as_time_space(replay):
        # take in a replay and output it in the format of:
        #
        # [  t1, t2 ... tn ],
        #
        # [[ x1, x2 ... xn ],
        #  [ y1, y2 ... yn ]]
        
        data = replay.as_list_with_timestamps()

        data = Replay.resample(data, 100)
        
        data = np.transpose(data)

        return data[0], data[1:]

    @staticmethod
    def spikiness(time, space):
        return np.std(Investigator.curvature(time, space))

    @staticmethod
    def jumpiness(time, space):
        return np.std(Investigator.velocity(time, space))

    @staticmethod
    def curvature(time, space):
        v = Investigator.velocity(time, space)
        a = Investigator.acceleration(time, space)

        v = np.transpose(np.transpose(v)[:-1])

        num = np.cross(v, a, axisa=0, axisb=0)
        # this is not necessary unless someone throws in a 3D replay or something.
        # num = np.linalg.norm(num, axis=0)

        den = np.linalg.norm(a, axis=0) ** 3

        k = np.divide(num, den, out=np.zeros_like(num), where=den != 0)
        
        return k
    
    @staticmethod
    def velocity(time, space):
        # differentiate both time and space so we get
        # [[ dx1, dx2 ... dxn ], =
        #  [ dy1, dy2 ... dyn ]] = [[ vx1, vx2 ... vxn ],
        # ---------------------- =  [ vy1, vy2 ... vyn ]]
        # [  dt1, dt2 ... dtn ]  =
        return np.diff(space) / np.diff(time)

    @staticmethod
    def acceleration(time, space):
        return np.diff(Investigator.velocity(time, space)) / np.diff(time)[:-1]
