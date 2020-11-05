(system 2
        ; Init: start at 00
        (and (not (var 0)) (not (var 1)))
        ; Trans:
        (or (and (not (var from 0)) (not (var from 1))      (var to 0)  (not (var to 1))) ;00->10
            (and      (var from 0)  (not (var from 1)) (not (var to 0))      (var to 1) ) ;10->01
            (and (not (var from 0))      (var from 1)  (not (var to 0)) (not (var to 1))) ;01->00
            (and (not (var from 0))      (var from 1)       (var to 0)       (var to 1) ) ;01->11
            (and      (var from 0)       (var from 1)  (not (var to 0))      (var to 1) ) ;11->01
        )
        (properties (AG (AF (var 0)))               ; True, var0 is always hit infinitely often
                    (AF (EG (var 1)))               ; True, we always eventually reach 01, from
                                                    ; where there is a path where v1 is always true
                    (AU (not (var 1)) (EG (var 1))) ; True, for all paths where v0 does not hold
                                                    ; until 01, from where we have a path where v1
                                                    ; always holds.
                    (EF (AG (var 1)))               ; False, no point is such that from there all paths
                                                    ; have v1 as true
                    (EU (not (var 0)) (EG (var 1))) ; False, for all paths starting from 00, v0 is
                                                    ; false for the first time at 10, where no path
                                                    ; is such that from there v1 always holds
                    (EF (and (var 0) (var 1)))      ; True, 11 is reachable
                    (EG (EF (and (var 0) (var 1)))) ; True, there are paths where 11 occurs
                                                    ; inifinitely often, that gives a sat model.
                    (AF (AG (or (not (var 0))
                                (not (var 1)))))    ; False, negation of the previous.
        )
)

