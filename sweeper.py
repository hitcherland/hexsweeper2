#!/usr/bin/python

from copy import deepcopy, copy
from operator import add
import functools
import random
import numpy

import logging

class Cell:
    def __init__( self, position ):
        self.position = position
        self.info = {}
        self.neighbours = set()

    def __hash__( self ):
        return hash( self.position )
    
    def __str__( self ):
        return str( self.position )

class Game:
    def __init__( self, **variables):
        self.states = {}
        self.info = {}
        self.variables = variables
        self.logger = logging.getLogger( 'game' ) 

        self.cells = self.make_grid()
        self.states = self.initialise_states( self.cells )
        self.update_info()

    def make_grid( self, hx ): pass
    def initialise_states( self ): pass
    def update_info( self, hx ): pass

class ClassicGame( Game ):
    ''' Classic Minesweeper:
        mines: placed randomly

        info #0: total number of cells 
        info #1: total number of mines
        info #2: positions of neighbours for each cell
        info #3: number of mined neighbours for each KNOWN cell
    '''

    def make_grid( self ):
        radius = self.variables[ 'radius' ]
        cells = {
            Cell( ( i, j ) )
            for i in range( -radius, radius + 1 )
            for j in range( -radius, radius + 1 )
            if abs( i + j ) <= radius
        }
        for cell in cells:
            neighbour_positions = {
                tuple( map( add, cell.position, dPosition ) )
                for dPosition in self.variables[ 'neighbours' ]
            }
            cell.neighbours = { c for c in cells if c.position in neighbour_positions }
        return cells

    def initialise_states( self, cells ):
        mineRatio = self.variables[ 'mineRatio' ]
        mine_count = round( mineRatio * len( cells ) )

        return {
            'mine': set( random.sample( cells, mine_count ) ),
            'known': set(),
            'flagged': set(),
            'unknown': cells.copy()
        }

    def update_info( self ):
        self.info = {
            'total cells': len( self.cells ),
            'remaining mines': len( self.states[ 'mine' ] - self.states[ 'known' ] ),
        }

        for state, cells in self.states.items():
            func_name = 'info_{}'.format( state )
            if hasattr( self, func_name ):
                for cell in cells:
                    getattr( self, func_name )( cell )

    def info_known( self, cell ):
        cell.info = {
            'neighbour mine count': len( cell.neighbours & self.states[ 'mine' ] ),
            'remaining neighbour mines': len( cell.neighbours & self.states[ 'mine' ] ) - len( cell.neighbours & self.states[ 'flagged' ] ),
            'unknown neighbours': cell.neighbours & self.states[ 'unknown' ]
        }

    def autocomplete( self, level = 0 ):
        self.update_info()

        ## if there are no mines remaining, we are done
        if self.info[ 'remaining mines' ] == 0:
            self.logger.info( 'no mines remaining' )
            return 'DONE'

        ## if all the remaining cells must be mines, we are done
        if self.info[ 'remaining mines' ] == len( self.states[ 'unknown' ] ):
            self.logger.info( 'Equal amount of remaining mines %d vs %d', self.info[ 'remaining mines' ], len( self.states[ 'unknown' ] ) )
            while self.states[ 'unknown' ]:
                self.states[ 'known' ].add( self.states[ 'unknown' ].pop() )
            return 'DONE'

        old_known = None
        while old_known != self.states[ 'known' ]:
            self.update_info()
            old_known = self.states[ 'known' ].copy()
            for cell in old_known:
                if not cell.info[ 'unknown neighbours' ]:
                    continue

                ## if we have no mines near us, add neighbours
                if cell.info[ 'remaining neighbour mines' ] == 0:
                    self.logger.info( 'cell at %s has no neighbouring mines', cell )
                    self.states[ 'unknown' ] -= cell.info[ 'unknown neighbours' ]
                    self.states[ 'known' ] |= cell.info[ 'unknown neighbours' ]
                
                if level >= 1:
                    # if we have 'n' unknown neighbours, and 'n' unknown mines left,
                    # then all  the neighbours must be mines

                    if cell.info[ 'remaining neighbour mines' ] == len( cell.info[ 'unknown neighbours' ] ):
                        self.logger.info( 'cell at %s as many remaining mines as unknown neighbours: %d', cell, cell.info[ 'remaining neighbour mines' ] )
                        self.states[ 'unknown' ] -= cell.info[ 'unknown neighbours' ]
                        if cell.info[ 'remaining neighbour mines' ] > 0:
                            self.states[ 'flagged' ] |= cell.info[ 'unknown neighbours' ]
                        else:
                            self.states[ 'known' ] |= cell.info[ 'unknown neighbours' ]

        if level >= 2: 
            old_known = None
            while old_known != self.states[ 'known' ]:
                self.update_info()
                old_known = self.states[ 'known' ].copy()
                self.implicit_solve_1()

        return 'STEP'

    def implicit_solve_1( self ):
        self.update_info()
        known = self.states[ 'known' ].copy()
        for cell in known:
            if not cell.info[ 'unknown neighbours' ]:
                continue
        
            superset = cell.info[ 'unknown neighbours' ]
            outer_mines = cell.info[ 'remaining neighbour mines' ] 
            for other_cell in known:
                if not other_cell.info[ 'unknown neighbours' ]:
                    continue
                subset = other_cell.info[ 'unknown neighbours' ]
                if other_cell == cell or not superset.issuperset( subset ):
                    continue

                inner_mines = other_cell.info[ 'remaining neighbour mines' ] 
                if outer_mines == inner_mines or len( superset - subset ) == outer_mines - inner_mines:
                    knowables = superset - subset
                    self.states[ 'unknown' ] -= knowables
                    if outer_mines - inner_mines > 0:
                        self.logger.info( 'implicit that cells near %s are mines because of neighbour overlap with %s', cell, other_cell )
                        self.states[ 'flagged' ] |= knowables 
                    else:
                        self.logger.info( 'implicit that cells near %s are not mines because of neighbour overlap with %s', cell, other_cell )
                        self.states[ 'known' ] |= knowables 
                
    def display( self, use_states = True ):
        radius = self.variables[ 'radius' ]
        output = ''
        for i in range( -radius, radius + 1 ):
            output += ' ' * abs( i )
            for cell in sorted( filter( lambda cell: cell.position[ 0 ] == i, self.cells ), key = lambda k: k.position[ 1 ] ):
                if use_states:
                    if cell in self.states[ 'flagged' ]:
                        output += '\x1b[34;1m*\x1b[0m '
                    elif cell in self.states[ 'known' ]:
                        output += '\x1b[32;1m{}\x1b[0m '.format( cell.info[ 'neighbour mine count' ] )
                    elif cell in self.states[ 'unknown' ]:
                        output += '- '
                    elif cell in self.states[ 'mine' ]:
                        output += '\x1b[31;1m*\x1b[0m '
                else:
                    if cell in self.states[ 'mine' ]:
                        output += '\x1b[31;1m*\x1b[0m '
                    else:
                        l = len( cell.neighbours & self.states[ 'mine' ] )
                        output += '{} '.format( l )
 
            output += '\n'
        return output

if __name__ == '__main__':
    import argparse
    logging.basicConfig()
    picker = logging.getLogger( 'picker' ) 
    picker.setLevel( logging.INFO )
    game_logger = logging.getLogger( 'game' ) 
    game_logger.setLevel( logging.INFO )
    p = argparse.ArgumentParser()
    p.add_argument( '-r', default = 3, type=int )
    p.add_argument( '-R', default = 0.1, type=float )
    p.add_argument( '-l', default = 4, type=int )
    a = p.parse_args()

    hexGame = ClassicGame(
        radius = a.r,
        mineRatio = a.R,
        neighbours = [ (-1,1), (0,1), (1,0), (1,-1), (0,-1), (-1,0) ]
    )

    while hexGame.autocomplete( level = 10 ) != 'DONE':
        frees = list( hexGame.states[ 'unknown' ] - hexGame.states[ 'mine' ] )
        if not frees:
            print( 'No frees:', len( hexGame.states[ 'unknown' ] ), len(  hexGame.states[ 'mine' ] ) )
            break
        cell = random.choice( frees )
        picker.info( 'chose "free" cell %s', cell )
        hexGame.states[ 'unknown' ].remove( cell )
        hexGame.states[ 'known' ].add( cell )
        hexGame.update_info()
        old_known =  hexGame.states[ 'known' ]
        print( hexGame.display() )
        input( '...' )

    hexGame.update_info()
    print( hexGame.display( False ) )
    print( hexGame.display() )
    print( 'PASS' if hexGame.states[ 'mine' ] == hexGame.states[ 'flagged' ]  else 'FAIL' )
