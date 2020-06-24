-- Create the top level root node named 'root'.
rootNode = gr.node('root')

armor = gr.material({0.3, 0.2, 0.1}, {0.8, 0.8, 0.8}, 50.0)
flesh = gr.material({0.8, 0.6, 0.4}, {0.8, 0.8, 0.8}, 50.0)
white = gr.material({1, 1, 1}, {0.8, 0.8, 0.8}, 50.0)

-- torso
torsoM = gr.mesh('sphere', 'torsoM')
torsoM:scale(0.8, 1.2, 0.6)
torsoM:translate(0.0, 0.45, 0)
torsoM:set_material(armor)
rootNode:add_child(torsoM)

-- shoulder
shoulder = gr.node('shoulder')
shoulderM = gr.mesh('sphere', 'shoulderM')
shoulderM:scale(1.4, 0.3, 0.4)
shoulderM:translate(0, 1.5, 0)
shoulderM:set_material(armor)
shoulder:add_child(shoulderM)
rootNode:add_child(shoulder)

-- left upper arm
luaJ = gr.joint('luaJ', {-45, 0, 180}, {-45, 0, 45});
lua = gr.node('lua')
luaM = gr.mesh('sphere', 'luaM')
luaM:scale(0.24, 0.7, 0.2)
luaM:translate(1.4, 0.8, 0)
luaM:set_material(armor)
lua:add_child(luaM)
luaJ:add_child(lua)
shoulder:add_child(luaJ)

-- left forearm
lfaJ = gr.joint('lfaJ', {-45, 0, 180}, {-45, 0, 45});
lfa = gr.node('lfa')
lfaM = gr.mesh('sphere', 'lfaM')
lfaM:scale(0.15, 0.5, 0.1)
lfaM:translate(1.4, -0.4, 0)
lfaM:set_material(flesh)
lfa:add_child(lfaM)
lfaJ:add_child(lfa)
lua:add_child(lfaJ)

-- left hand
lhJ = gr.joint('lhJ', {-45, 0, 180}, {-45, 0, 45});
lh = gr.node('lh')
lhM = gr.mesh('sphere', 'lhM')
lhM:scale(0.15, 0.15, 0.15)
lhM:translate(1.4, -1.05, 0)
lhM:set_material(white)
lh:add_child(lhM)
lhJ:add_child(lh)
lfa:add_child(lhJ)

-- right upper arm
ruaJ = gr.joint('ruaJ', {-45, 0, 180}, {-45, 0, 45});
rua = gr.node('rua')
ruaM = gr.mesh('sphere', 'ruaM')
ruaM:scale(0.25, 0.7, 0.2)
ruaM:translate(-1.4, 0.8, 0)
ruaM:set_material(armor)
rua:add_child(ruaM)
ruaJ:add_child(rua)
shoulder:add_child(ruaJ)

-- right forearm
rfaJ = gr.joint('rfaJ', {-45, 0, 180}, {-45, 0, 45});
rfa = gr.node('rfa')
rfaM = gr.mesh('sphere', 'rfaM')
rfaM:scale(0.15, 0.5, 0.1)
rfaM:translate(-1.4, -0.4, 0)
rfaM:set_material(flesh)
rfa:add_child(rfaM)
rfaJ:add_child(rfa)
rua:add_child(rfaJ)

-- right hand
rhJ = gr.joint('rhJ', {-45, 0, 180}, {-45, 0, 45});
rh = gr.node('rh')
rhM = gr.mesh('sphere', 'rhM')
rhM:scale(0.15, 0.15, 0.15)
rhM:translate(-1.4, -1.05, 0)
rhM:set_material(white)
rh:add_child(rhM)
rhJ:add_child(rh)
rfa:add_child(rhJ)

-- hips
hips = gr.node('hips')
hipsM = gr.mesh('sphere', 'hipsM')
hipsM:scale(0.8, 0.4, 0.4)
hipsM:translate(0, -0.6, 0)
hipsM:set_material(armor)
hips:add_child(hipsM)
rootNode:add_child(hips)

-- left thigh
ltJ = gr.joint('ltJ', {-45, 0, 180}, {-45, 0, 45});
lt = gr.node('lt')
ltM = gr.mesh('sphere', 'ltM')
ltM:scale(0.3, 0.9, 0.3)
ltM:translate(0.5, -1.5, 0)
ltM:set_material(armor)
lt:add_child(ltM)
ltJ:add_child(lt)
hips:add_child(ltJ)

-- left calf
lcJ = gr.joint('lcJ', {-45, 0, 180}, {-45, 0, 45});
lc = gr.node('lc')
lcM = gr.mesh('sphere', 'lcM')
lcM:scale(0.2, 0.6, 0.2)
lcM:translate(0.5, -3, 0)
lcM:set_material(flesh)
lc:add_child(lcM)
lcJ:add_child(lc)
lt:add_child(lcJ)

-- left foot
lfJ = gr.joint('lfJ', {-45, 0, 180}, {-45, 0, 45});
lf = gr.node('lf')
lfM = gr.mesh('sphere', 'lfM')
lfM:scale(0.3, 0.1, 0.2)
lfM:translate(0.75, -3.65, 0)
lfM:set_material(white)
lf:add_child(lfM)
lfJ:add_child(lf)
lc:add_child(lfJ)

-- right thigh
rtJ = gr.joint('rtJ', {-45, 0, 180}, {-45, 0, 45});
rt = gr.node('rt')
rtM = gr.mesh('sphere', 'rtM')
rtM:scale(0.3, 0.9, 0.3)
rtM:translate(-0.5, -1.5, 0)
rtM:set_material(armor)
rt:add_child(rtM)
rtJ:add_child(rt)
hips:add_child(rtJ)

-- right calf
rcJ = gr.joint('rcJ', {-45, 0, 180}, {-45, 0, 45});
rc = gr.node('rc')
rcM = gr.mesh('sphere', 'rcM')
rcM:scale(0.2, 0.6, 0.2)
rcM:translate(-0.5, -3, 0)
rcM:set_material(flesh)
rc:add_child(rcM)
rcJ:add_child(rc)
rt:add_child(rcJ)

-- right foot
rfJ = gr.joint('rfJ', {-45, 0, 180}, {-45, 0, 45});
rf = gr.node('rf')
rfM = gr.mesh('sphere', 'rfM')
rfM:scale(0.3, 0.1, 0.2)
rfM:translate(-0.75, -3.65, 0)
rfM:set_material(white)
rf:add_child(rfM)
rfJ:add_child(rf)
rc:add_child(rfJ)

-- neck
neckJ = gr.joint('neckJ', {-45, 0, 180}, {-45, 0, 45});
neck = gr.node('neck')
neckM = gr.mesh('sphere', 'neckM')
neckM:scale(0.2, 0.3, 0.2)
neckM:translate(0, 1.9, 0)
neckM:set_material(flesh)
neck:add_child(neckM)
neckJ:add_child(neck)
shoulder:add_child(neckJ)

-- head
headJ = gr.joint('headJ', {-45, 0, 180}, {-45, 0, 45});
head = gr.node('head')
headM = gr.mesh('sphere', 'neckM')
headM:scale(0.6, 0.6, 0.6)
headM:translate(0, 2.6, 0)
headM:set_material(white)
head:add_child(headM)
headJ:add_child(head)
neck:add_child(headJ)

-- rootNode:scale(8.0, 8.0, 8.0)

-- Return the root with all of it's childern.  The SceneNode A3::m_rootNode will be set
-- equal to the return value from this Lua script.
return rootNode
