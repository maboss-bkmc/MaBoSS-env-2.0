import cmaboss
import numpy as np
from unittest import TestCase

class TestCMaBoSS(TestCase):

    def test_load_model(self):
        net = cmaboss.MaBoSSNet(network="../examples/metastasis.bnd")
        cfg = cmaboss.MaBoSSCfg(net, "../examples/metastasis.cfg")
        sim = cmaboss.MaBoSSSim(net=net, cfg=cfg)
        res = sim.run()

    def test_load_model_cellcycle(self):
        net = cmaboss.MaBoSSNet(network="../tests/cellcycle/cellcycle.bnd")
        cfg = cmaboss.MaBoSSCfg(net, "../tests/cellcycle/cellcycle_runcfg.cfg", "../tests/cellcycle/cellcycle_runcfg-thread_6.cfg")
        sim = cmaboss.MaBoSSSim(net=net, cfg=cfg)
        res = sim.run()

    def test_simulate(self):
        sim = cmaboss.MaBoSSSim(network="../examples/metastasis.bnd", config="../examples/metastasis.cfg")
        res = sim.run()
        res.get_probtraj()
        res.get_last_probtraj()
        res.get_nodes_probtraj()
        res.get_last_nodes_probtraj()
        res.get_fp_table()
        
    def test_load_model_error(self):
        with self.assertRaises(cmaboss.BNException):
            cmaboss.MaBoSSSim(network="../examples/metastasis-error.bnd", config="../examples/metastasis.cfg")

    def test_last_states(self):
        expected = {
            '<nil>': 0.206, 'Apoptosis -- CellCycleArrest': 0.426, 'CellCycleArrest': 0.068, 
            'Migration -- Metastasis -- Invasion -- CellCycleArrest': 0.3
        }

        expected_nodes = {
            'Metastasis': 0.3, 'Invasion': 0.3, 'Migration': 0.3, 'Apoptosis': 0.426, 'CellCycleArrest': 0.794
        }

        sim = cmaboss.MaBoSSSim(network="../examples/metastasis.bnd", config="../examples/metastasis.cfg")
        res = sim.run(only_last_state=False)

        raw_res, states, _ = res.get_last_probtraj()
        for i, value in enumerate(np.nditer(raw_res)):
            self.assertAlmostEqual(value, expected[states[i]])
        
        raw_nodes_res, nodes, _ = res.get_last_nodes_probtraj()
        for i, value in enumerate(np.nditer(raw_nodes_res)):
            self.assertAlmostEqual(value, expected_nodes[nodes[i]])
        
        simfinal = cmaboss.MaBoSSSim(network="../examples/metastasis.bnd", config="../examples/metastasis.cfg")
        resfinal = simfinal.run(only_last_state=True)
    
        raw_res, states, _ = res.get_last_probtraj()
        for i, value in enumerate(np.nditer(raw_res)):
            self.assertAlmostEqual(value, expected[states[i]])
        
        raw_nodes_res, nodes, _ = res.get_last_nodes_probtraj()
        for i, value in enumerate(np.nditer(raw_nodes_res)):
            self.assertAlmostEqual(value, expected_nodes[nodes[i]])
           
    def test_load_model_str(self):
        with open("../examples/metastasis.bnd", "r") as bnd, open("../examples/metastasis.cfg", "r") as cfg:    
            sim = cmaboss.MaBoSSSim(network_str=bnd.read(),config_str=cfg.read())
            res = sim.run()
            res.get_probtraj()
            res.get_last_probtraj()
            res.get_nodes_probtraj()
            res.get_last_nodes_probtraj()
            res.get_fp_table()

    def test_load_model_str_error(self):
        with open("../examples/metastasis-error.bnd", "r") as bnd, open("../examples/metastasis.cfg", "r") as cfg:    
            with self.assertRaises(cmaboss.BNException):
                cmaboss.MaBoSSSim(network_str=bnd.read(),config_str=cfg.read())
