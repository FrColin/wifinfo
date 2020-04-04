# module téléinformation client
# rene-d 2020

# Simulation de Téléinformation Client

import datetime
import json
import math
import random
import time


def group(label, value):
    """
    Construit un groupe d'une trame de téléinformation
    """
    cksum = 0
    for c in label:
        cksum += ord(c)
    cksum += ord(" ")
    for c in value:
        cksum += ord(c)
    cksum = (cksum & 63) + 32
    return "\x0A" + label + " " + str(value) + " " + chr(cksum) + "\x0D"


class SimuTic:
    """
    Simule un compteur en option HP/HC
    """

    def __init__(self):
        self.intervalle = 0
        self.heures_creuses = False
        self._hchc = 52000000
        self._hchp = 49000000
        self._hcjb = 65808
        self._hpjb = 112678
        self._hcjw = 28728
        self._hpjw = 12592
        self._hcjr = 57793
        self._hpjr = 18012
        self._iinst = 0
        self._isousc = 30
        self._adps = 0
        self._adco = "012345678123"
        self._pseuil = 0
        self.periode = "B"

    def bascule(self, quoi="T"):
        """
        Bascule de période tarifaire
        """
        if quoi == "T":
            self.heures_creuses = not self.heures_creuses
        elif quoi == "A":
            if self.adps != 0:
                self.adps = 0
            else:
                self.adps = self.isousc + 1
        elif quoi == "P":
            self._pseuil = 4000 - self._pseuil
        elif quoi == "B":
            self.periode = quoi
        elif quoi == "W":
            self.periode = quoi
        elif quoi == "R":
            self.periode = quoi

    @property
    def timestamp(self):
        """
        Retourne l'heure courante au format ISO8601
        """
        return datetime.datetime.now().strftime("%Y-%m-%dT%H:%M:%SZ")

    @property
    def adco(self):
        """
        Retourne le numéro de compteur
        """
        return self._adco

    @adco.setter
    def adco(self, value):
        """
        Positionne le numéro de compteur
        """
        value = str(value)[0:12]
        self._adco = "0" * (12 - len(value)) + value

    @property
    def iinst_raw(self):
        """"
        Calcule l'intensité instantanée et met à jour les index et la puissance apparente
        """
        return f"{self.iinst:03d}"

    @property
    def iinst(self):
        """"
        Calcule l'intensité instantanée et met à jour les index et la puissance apparente
        """
        now = time.monotonic()
        self._iinst = 20 * abs(math.sin(now / 100))
        delta = (now - self.intervalle) * self.papp / 3600.0
        self.intervalle = now
        if self.heures_creuses:
            self._hchc += delta
        else:
            self._hchp += delta
        return round(self._iinst)

    @property
    def papp_raw(self, text=False):
        """
        Calcule la Puissance APParente en VA avec un cos φ aléatoire
        """
        return f"{self.papp:05d}"

    @property
    def papp(self):
        """
        Calcule la Puissance APParente en VA avec un cos φ aléatoire
        """
        cos_phi = random.uniform(0.85, 1.15)
        return round(self._iinst * 230 * cos_phi) + self._pseuil

    @property
    def hchc_raw(self):
        """
        Retourne l'index Heures Creuses
        """
        return f"{self.hchc:09d}"

    @property
    def hchc(self):
        """
        Retourne l'index Heures Creuses
        """
        return round(self._hchc)

    @property
    def hchp_raw(self, text=False):
        """
        Retourne l'index Heures Pleines
        """
        return f"{self.hchp:09d}"

    @property
    def hchp(self, text=False):
        """
        Retourne l'index Heures Pleines
        """
        return round(self._hchp)
    @property
    def hcjb_raw(self, text=False):
            return f"{self._hcjb:09d}"
    @property
    def hpjb_raw(self, text=False):
            return f"{self._hpjb:09d}"
    @property
    def hcjw_raw(self, text=False):
            return f"{self._hcjw:09d}"
    @property
    def hpjw_raw(self, text=False):
            return f"{self._hpjw:09d}"
    @property
    def hcjr_raw(self, text=False):
            return f"{self._hcjr:09d}"
    @property
    def hpjr_raw(self, text=False):
            return f"{self._hpjr:09d}"

    @property
    def ptec_raw(self):
        """
        Retourne la Période Tarifaire En Cours
        """
        return (self.ptec + "....")[:4]

    @property
    def ptec(self):
        """
        Retourne la Période Tarifaire En Cours
        """
        if self.periode == "B":
            return "HCJB" if self.heures_creuses else "HPJB"
        elif self.periode == "W":
            return  "HCJW" if self.heures_creuses else "HPJW"
        elif self.periode == "R":
            return  "HCJR" if self.heures_creuses else "HPJR"
       
    @property
    def adps_raw(self):
        """
        Retourne l'Avertissement de Dépassement de Puissance Souscrite
        """
        if self._adps == 0:
            return None
        return f"{round(self._adps):03d}"

    @property
    def adps(self):
        return self._adps

    @adps.setter
    def adps(self, value):
        """
        Positionne l'Avertissement de Dépassement de Puissance Souscrite
        """
        if isinstance(value, bool):
            self._adps = [0, self._isousc + 1][value]
        elif not (self._isousc <= value <= 999):
            self._adps = 0
        else:
            self._adps = value

    @property
    def isousc_raw(self):
        """
        Retourne l'Intensité SOUSCrite
        """
        return f"{self._isousc:02d}"

    @property
    def isousc(self):
        """
        Retourne l'Intensité SOUSCrite
        """
        return self._isousc

    def trame(self):
        """
        Construit une trame telle qu'elle esr envoyée par un compteur
        """
        iinst_raw = self.iinst_raw
        tinfo = "\x02"
        tinfo += group("ADCO", self.adco)
        tinfo += group("OPTARIF", "BBR(")
        tinfo += group("ISOUSC", self.isousc_raw)
       # tinfo += group("HCHC", self.hchc_raw)
       # tinfo += group("HCHP", self.hchp_raw)
        tinfo += group("BBRHCJB", self.hcjb_raw)
        tinfo += group("BBRHPJB", self.hpjb_raw)
        tinfo += group("BBRHCJW", self.hcjw_raw)
        tinfo += group("BBRHPJW", self.hpjw_raw)
        tinfo += group("BBRHCJR", self.hcjr_raw)
        tinfo += group("BBRHPJR", self.hpjr_raw)
        tinfo += group("PTEC", self.ptec_raw)
        tinfo += group("DEMAIN", "----")
        tinfo += group("IINST", iinst_raw)
        tinfo += group("IMAX", "042")
        tinfo += group("PAPP", self.papp_raw)
        if self.adps != 0:
            tinfo += group("ADPS", self.adps_raw)
        tinfo += group("HHPHC", "D")
        tinfo += group("MOTDETAT", "000000")
        tinfo += "\x03"

        return tinfo.encode("ascii")

    def json_dict(self):
        """
        Retourne les valeurs sous forme de dictionnaire JSON
        """
        iinst = self.iinst
        d = {
            "timestamp": self.timestamp,
            "ADCO": self.adco,
            "OPTARIF": "BBR(",
            "ISOUSC": self.isousc,
            # "HCHC": self.hchc,
            # "HCHP": self.hchp,
            "BBRHCJB": self._hcjb,
            "BBRHPJB": self._hpjb,
            "BBRHCJW": self._hcjw,
            "BBRHPJW": self._hpjw,
            "BBRHCJR": self._hcjr,
            "BBRHPJR": self._hpjr,
            "PTEC": self.ptec,
            "IINST": iinst,
            "IMAX": 42,
            "PAPP": self.papp,
            "HHPHC": "D",
            "MOTDETAT": 0,
        }
        if self.adps != 0:
            d["ADPS"] = self.adps
        return json.dumps(d, separators=(",", ":"))

    def json_array(self):
        """
        Retourne les valeurs sous forme de tableau JSON
        """
        iinst_raw = self.iinst_raw
        d = [
            {"na": "timestamp", "va": self.timestamp},
            {"na": "ADCO", "va": self.adco},
            {"na": "OPTARIF", "va": "BBR("},
            {"na": "ISOUSC", "va": self.isousc_raw},
            # {"na": "HCHC", "va": self.hchc_raw},
            # {"na": "HCHP", "va": self.hchp_raw},
            {"na":"BBRHCJB", "va": self.hcjb_raw},
            {"na":"BBRHPJB", "va": self.hpjb_raw},
            {"na":"BBRHCJW", "va": self.hcjw_raw},
            {"na":"BBRHPJW", "va": self.hpjw_raw},
            {"na":"BBRHCJR", "va": self.hcjr_raw},
            {"na":"BBRHPJR", "va": self.hpjr_raw},
            {"na": "PTEC", "va": self.ptec_raw},
            {"na": "IINST", "va": iinst_raw},
            {"na": "IMAX", "va": "042"},
            {"na": "PAPP", "va": self.papp_raw},
            {"na": "HHPHC", "va": "D"},
            {"na": "MOTDETAT", "va": "000000"},
        ]
        if self.adps != 0:
            d.append({"na": "ADPS", "va": self.adps_raw})
        return json.dumps(d, indent=4)


tic = SimuTic()
