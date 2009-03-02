// -*- C++ -*-
#ifndef PROXIMITY_DATABASE_H
#define PROXIMITY_DATABASE_H

#include "Vec3.h"

#include <boost/tuple/tuple.hpp>

#include <map>
#include <vector>


template <typename T>
class ProximityDatabase
{
public:
    typedef boost::tuple<T, unsigned int, unsigned int> StoredType;

    class TokenType
    {
    public:
        ~TokenType()
            { m_db->Erase(*this); }

        void UpdatePosition(const OpenSteer::Vec3& p)
            { m_db->UpdatePosition(*this, p); }

    private:
        TokenType(const StoredType& object, std::size_t index, ProximityDatabase& db) :
            m_object(object),
            m_old_index(index),
            m_db(&db)
            {}

        StoredType m_object;
        std::size_t m_old_index;
        ProximityDatabase* m_db;

        friend class ProximityDatabase<T>;
    };

    ProximityDatabase(const OpenSteer::Vec3& center,
                      float dimensions,
                      std::size_t cells_per_side) :
        m_origin(center - OpenSteer::Vec3(dimensions * 0.5f,
                                          dimensions * 0.5f,
                                          dimensions * 0.5f)),
        m_dimensions(dimensions),
        m_cell_dimensions(dimensions / cells_per_side),
        m_cells_per_side(cells_per_side),
        m_grid_cells(m_cells_per_side * m_cells_per_side * m_cells_per_side)
        {}

    TokenType* Insert(T t, unsigned int type_flags = -1, unsigned int empire_ids = -1)
        {
            StoredType stored_val(t, type_flags, empire_ids);
            std::size_t index = GridIndexOf(t->position());
            m_grid_cells[index][t] = stored_val;
            return new TokenType(stored_val, index, *this);
        }

    void FindAll(std::vector<T>& results,
                 unsigned int type_flags = -1,
                 unsigned int empire_ids = -1)
        {
            for (std::size_t i = 0; i < m_grid_cells.size(); ++i) {
                for (typename std::map<T, StoredType>::iterator it = m_grid_cells[i].begin();
                     it != m_grid_cells[i].end();
                     ++it) {
                    if (type_flags & boost::get<1>(it->second) &&
                        empire_ids & boost::get<1>(it->second)) {
                        results.push_back(it->first);
                    }
                }
            }
        }

    void FindInRadius(const OpenSteer::Vec3& center,
                      const float radius,
                      std::vector<T>& results,
                      unsigned int type_flags = -1,
                      unsigned int empire_ids = -1)
        { FindInRadiusImpl(center, radius, results, type_flags, empire_ids, false); }

    T FindNearestInRadius(const OpenSteer::Vec3& center,
                          const float radius,
                          unsigned int type_flags = -1,
                          unsigned int empire_ids = -1)
        {
            std::vector<T> results;
            FindInRadiusImpl(center, radius, results, type_flags, empire_ids, true);
            return results[0];
        }

    T FindNearest(const OpenSteer::Vec3& center,
                  unsigned int type_flags = -1,
                  unsigned int empire_ids = -1)
        {
            // TODO: Consider adding an initial pass that looks in some default
            // nearby range first -- then fails over to a full search -- as an
            // optimization.
            T retval = 0;
            float nearest_dist_squared = FLT_MAX;
            for (std::size_t i = 0; i < m_grid_cells.size(); ++i) {
                for (typename std::map<T, StoredType>::iterator it = m_grid_cells[i].begin();
                     it != m_grid_cells[i].end();
                     ++it) {
                    if (type_flags & boost::get<1>(it->second) &&
                        empire_ids & boost::get<2>(it->second)) {
                        float dist_squared =
                            (center - it->first->position()).lengthSquared();
                        if (dist_squared < nearest_dist_squared) {
                            nearest_dist_squared = dist_squared;
                            retval = it->first;
                        }
                    }
                }
            }
            return retval;
        }

private:
    void UpdatePosition(TokenType& token, const OpenSteer::Vec3& p)
        {
            std::size_t old_index = token.m_old_index;
            std::size_t new_index = GridIndexOf(p);
            if (old_index != new_index) {
                m_grid_cells[old_index].erase(boost::get<0>(token.m_object));
                m_grid_cells[new_index][boost::get<0>(token.m_object)] = token.m_object;
                token.m_old_index = new_index;
            }
        }

    void Erase(const TokenType& token)
        {
            assert(token.m_old_index < m_grid_cells.size());
            assert(m_grid_cells[token.m_old_index].find(boost::get<0>(token.m_object)) !=
                   m_grid_cells[token.m_old_index].end());
            m_grid_cells[token.m_old_index].erase(boost::get<0>(token.m_object));
        }

    void GridIndicesOf(const OpenSteer::Vec3& vec,
                       std::size_t& x_index,
                       std::size_t& y_index,
                       std::size_t& z_index)
        {
            OpenSteer::Vec3 rel_pos(vec - m_origin);
            if (rel_pos.x < 0 || m_dimensions < rel_pos.x)
                x_index = 0;
            else
                x_index = static_cast<std::size_t>(rel_pos.x / m_cell_dimensions);
            if (rel_pos.y < 0 || m_dimensions < rel_pos.y)
                y_index = 0;
            else
                y_index = static_cast<std::size_t>(rel_pos.y / m_cell_dimensions);
            if (rel_pos.z < 0 || m_dimensions < rel_pos.z)
                z_index = 0;
            else
                z_index = static_cast<std::size_t>(rel_pos.z / m_cell_dimensions);
        }

    std::size_t GridIndexOf(const OpenSteer::Vec3& vec)
        {
            std::size_t retval = 0;
            std::size_t x_index;
            std::size_t y_index;
            std::size_t z_index;
            GridIndicesOf(vec, x_index, y_index, z_index);
            retval =
                x_index * m_cells_per_side * m_cells_per_side +
                y_index * m_cells_per_side +
                z_index;
            return retval;
        }

    std::size_t GridIndexOf(std::size_t x_index, std::size_t y_index, std::size_t z_index)
        {
            return
                x_index * m_cells_per_side * m_cells_per_side +
                y_index * m_cells_per_side +
                z_index;
        }

    void FindInRadiusImpl(const OpenSteer::Vec3& center,
                          const float radius,
                          std::vector<T>& results,
                          unsigned int type_flags,
                          unsigned int empire_ids,
                          bool find_nearest)
        {
            if (find_nearest) {
                results.resize(1);
                results[0] = 0;
            }

            std::size_t grid_radius =
                static_cast<std::size_t>(std::ceil(radius / m_cell_dimensions));
            std::size_t x_index;
            std::size_t y_index;
            std::size_t z_index;
            GridIndicesOf(center, x_index, y_index, z_index);

            std::size_t x_begin = x_index - grid_radius;
            std::size_t x_end = x_index + grid_radius;
            if (x_index < x_begin)
                x_begin = 0;
            else if (m_cells_per_side <= x_end)
                x_end = m_cells_per_side - 1;

            std::size_t y_begin = y_index - grid_radius;
            std::size_t y_end = y_index + grid_radius;
            if (y_index < y_begin)
                y_begin = 0;
            else if (m_cells_per_side <= y_end)
                y_end = m_cells_per_side - 1;

            std::size_t z_end = z_index + grid_radius;
            std::size_t z_begin = z_index - grid_radius;
            if (z_index < z_begin)
                z_begin = 0;
            else if (m_cells_per_side <= z_end)
                z_end = m_cells_per_side - 1;

            float nearest_dist_squared = FLT_MAX;
            for (std::size_t x = x_begin; x < x_end; ++x) {
                for (std::size_t y = y_begin; y < y_end; ++y) {
                    for (std::size_t z = z_begin; z < z_end; ++z) {
                        std::map<T, StoredType>& cell = m_grid_cells[GridIndexOf(x, y, z)];
                        for (typename std::map<T, StoredType>::iterator it = cell.begin();
                             it != cell.end();
                             ++it) {
                            if (type_flags & boost::get<1>(it->second) &&
                                empire_ids & boost::get<2>(it->second)) {
                                if (find_nearest) {
                                    float dist_squared =
                                        (center - it->first->position()).lengthSquared();
                                    if (dist_squared < nearest_dist_squared) {
                                        nearest_dist_squared = dist_squared;
                                        results[0] = it->first;
                                    }
                                } else {
                                    results.push_back(it->first);
                                }
                            }
                        }
                    }
                }
            }
        }

    OpenSteer::Vec3 m_origin;
    float m_dimensions;
    float m_cell_dimensions;
    std::size_t m_cells_per_side;
    std::vector<std::map<T, StoredType> > m_grid_cells;

    friend class TokenType;
};

#endif
